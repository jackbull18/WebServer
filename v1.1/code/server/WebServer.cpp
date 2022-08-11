/**
 * @file WebServer.cpp
 * @author jack
 * @brief 简单服务器的实现
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "WebServer.h"
#include <iostream>

WebServer::~WebServer(){
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SQLConnPool::instance()->closeConnPool();
}

void WebServer::init(int port, int timeoutMS, bool openLinger, 
                     int sqlPort, const char* sqlUser, const  char* sqlPwd, 
                     const char* dbName, int connPoolNum, int threadNum,
                     bool openLog, int logLevel, int logQueSize,int trigMode)
{
    port_ = port;
    timeoutMS_ = timeoutMS;
    openLinger_  = openLinger;
    isClose_ = false;
    threadNum_ = threadNum;
    sqlConnNum_ = connPoolNum;

    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/",16);

    /* HTTPConn类静态变量 */
    HTTPConn::userCount = 0;
    HTTPConn::srcDir = srcDir_;

    /* 初始化unqiue_ptr */
    timer_ = std::make_unique<HeapTimer>(64);
    threadpool_ = std::make_unique<ThreadPool>(threadNum);
    epoller_ = std::make_unique<Epoller>();

    /* 数据库初始化 */
    SQLConnPool::instance()->init("localhost",sqlPort,sqlUser,sqlPwd,dbName,connPoolNum);


    /* 初始化日志 */
    if(openLog){
        initLog_(logLevel, logQueSize);
    }
    
    /* 事件模式初始化 */
    initEventMode_(trigMode);

    /* 监听套接字 */
    if(!initSocket_()){
        isClose_ = true;
    }

    if(isClose_){
        LOG_ERROR("========== Server init error!==========");
    }else{
        LOG_INFO("========== Server init ==========");
        LOG_INFO("Port:%d, OpenLinger: %s", port_, openLinger_? "true":"false");
        LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                        (listenEvent_ & EPOLLET ? "ET": "LT"),
                        (connEvent_ & EPOLLET ? "ET": "LT"));
        LOG_INFO("srcDir: %s", srcDir_);
        LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", sqlConnNum_, threadNum_);
    }
}

void WebServer::init(Config& config){
    init(config.port, 
         config.timeoutMS, 
         config.openLinger,  
         config.sqlPort, 
         config.sqlUser.c_str(), 
         config.sqlPwd.c_str(), 
         config.dbName.c_str(), 
         config.connPoolNum, 
         config.threadNum,
         config.openLog, 
         config.logLevel, 
         config.logQueSize,
         config.trigMode);

}

void WebServer::start(){
    int timeMS = -1;
    if(!isClose_){
        LOG_INFO("============ Server start =============");
    }
    while(!isClose_){
        LOG_DEBUG("enter while loop")
        if(timeoutMS_ > 0){
            LOG_DEBUG("get tick");
            timeMS = timer_->getNextTick();
            LOG_DEBUG("get tick success!");
        }
        int eventCnt = epoller_->wait(timeMS);
        for(int i = 0; i < eventCnt; i++){
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            if(fd == listenFd_){
                LOG_DEBUG("enter listen!");
                dealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                LOG_DEBUG("enter close!");
                assert(users_.count(fd) > 0);
                closeConn_(&users_[fd]);
            }else if(events & EPOLLIN){
                LOG_DEBUG("enter read!");
                assert(users_.count(fd) > 0);
                dealRead_(&users_[fd]);
            }else if(events & EPOLLOUT){
                LOG_DEBUG("enter write!");
                assert(users_.count(fd) > 0);
                dealWrite_(&users_[fd]);
            }else{
                LOG_ERROR("Unexpected event!");
            }
        }
    }
}

bool WebServer::initSocket_(){
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024){
        LOG_ERROR("Port:%d error!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    struct linger optLinear = {0};
    if(openLinger_){
        optLinear.l_onoff = 1;
        optLinear.l_linger = 1;
    } 
    /* 创建监听socket */
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0){
        LOG_ERROR("Create socket error!");
        return false;
    }
    /* 设置监听描述符关闭方式 */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinear, sizeof(optLinear));
    if(ret < 0){
        LOG_ERROR("Init linger error!");
        close(listenFd_);
        return false;
    }
    /* 设置端口复用 */
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
    if(ret == -1){
        LOG_ERROR("Set Port-reuse error!");
        close(listenFd_);
        return false;
    }

    /* 绑定监听描述符和地址 */
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret  < 0){
        LOG_ERROR("Bind port %d failed!", port_);
        close(listenFd_);
        return false;
    }

    /* 开始监听 */
    ret = listen(listenFd_, 8);
    if(ret < 0){
        LOG_ERROR("Listen port %d failed!", port_);
        close(listenFd_);
        return false;
    }

    /* 交付epoll管理监听事件 */
    ret = epoller_->addFd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret == 0){
        LOG_ERROR("Epoller add listen error!");
        close(listenFd_);
        return false;
    }
    LOG_INFO("Epoller add listen success!");

    /* 设置监听非阻塞 */
    setFdNonblock_(listenFd_);
    LOG_INFO("Server port: %d init successfully!", port_)
    return true;
}

void WebServer::initEventMode_(int trigMode){
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HTTPConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::initLog_(int logLevel, int logQueSize){
    Log::Instance()->init(logLevel,"./log", ".log", logQueSize);
    LOG_INFO("============= Log init =============");
    LOG_INFO("LogSys level: %d", logLevel);
}

void WebServer::addClient_(int fd, sockaddr_in addr){
    LOG_DEBUG("enter add client!");
    LOG_DEBUG("conn fd: %d",fd);
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeoutMS_ > 0){
        timer_->addNode(fd, timeoutMS_, std::bind(&WebServer::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | connEvent_);
    setFdNonblock_(fd);
    LOG_INFO("Client[%d] in!", users_[fd].getFd());
}

void WebServer::dealListen_(){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do{
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd <= 0){
            return;
        }else if(HTTPConn::userCount >= MAX_FD){
            sendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        addClient_(fd, addr);
    }while(listenEvent_ & EPOLLET);
}

void WebServer::dealRead_(HTTPConn* client){
    assert(client);
    addSurvivalTime_(client);
    threadpool_->appendTask(std::bind(&WebServer::onRead_, this, client));
}

void WebServer::dealWrite_(HTTPConn* client){
    assert(client);
    addSurvivalTime_(client);
    threadpool_->appendTask(std::bind(&WebServer::onWrite_, this, client));
}

void WebServer::onRead_(HTTPConn* client){
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN){
        closeConn_(client);
        return;
    }
    onProcess_(client);
}

void WebServer::onWrite_(HTTPConn* client){
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->toWriteBytes() == 0){
        if(client->isKeepAlive()){
            onProcess_(client);
            return;
        }
    }else if(ret < 0){
        if(writeErrno == EAGAIN){
            epoller_->modfildFd(client->getFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    closeConn_(client);
}

void WebServer::onProcess_(HTTPConn* client){
    if(client->process()){
        epoller_->modfildFd(client->getFd(), connEvent_ | EPOLLOUT);
    }else{
        epoller_->modfildFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::closeConn_(HTTPConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!",client->getFd());
    epoller_->deleteFd(client->getFd());
    client->closeConn();
}

int WebServer::setFdNonblock_(int fd){
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
void WebServer::sendError_(int fd, const char* info){
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    close(fd);
}
void WebServer::addSurvivalTime_(HTTPConn* client){
    assert(client);
    if(timeoutMS_ > 0){
        timer_->adjustNode(client->getFd(), timeoutMS_);
    }
}

