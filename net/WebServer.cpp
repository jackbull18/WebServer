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

WebServer::~WebServer(){
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SQLConnPool::instance()->closeConnPool();
}

void WebServer::init(int port, int timeoutMS, bool OptLinger, 
                     int sqlPort, const char* sqlUser, const  char* sqlPwd, 
                     const char* dbName, int connPoolNum, int threadNum,
                     bool openLog, int logLevel, int logQueSize)
{
    
}

void WebServer::start(){
    int timeMS = -1;
    while(!isClose_){
        if(timeoutMS_ > 0){
            timeMS = timer_->getNextTick();
        }
        int eventCnt = epoller_->wait(timeMS);
        for(int i = 0; i < eventCnt; i++){
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            if(fd == listenFd_){dealListen_();}
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                assert(users_.count(fd) > 0);
                closeConn_(&users_[fd]);
            }else if(events & EPOLLIN){
                assert(users_.count(fd) > 0);
                dealRead_(&users_[fd]);
            }else if(events & EPOLLOUT){
                assert(users_.count(fd) > 0);
                dealWrite_(&users_[fd]);
            }
        }
    }
}

bool WebServer::initSocket_(){
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    struct linger optLinear = {0};
    optLinear.l_onoff = 1;
    optLinear.l_linger = 1;
    /* 创建监听socket */
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0){
        return false;
    }
    /* 设置监听描述符关闭方式 */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinear, sizeof(optLinear));
    if(ret < 0){
        close(listenFd_);
        return false;
    }
    /* 设置端口复用 */
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
    if(ret == -1){
        close(listenFd_);
        return false;
    }

    /* 绑定监听描述符和地址 */
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret  < 0){
        close(listenFd_);
        return false;
    }

    /* 开始监听 */
    ret = listen(listenFd_, 8);
    if(ret < 0){
        close(listenFd_);
        return false;
    }

    /* 交付epoll管理监听事件 */
    ret = epoller_->addFd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret == 0){
        close(listenFd_);
        return false;
    }

    /* 设置监听非阻塞 */
    setFdNonblock_(listenFd_);
    return true;
}

void WebServer::addClient_(int fd, sockaddr_in addr){
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeoutMS_ > 0){
        timer_->addNode(fd, timeoutMS_, std::bind(&WebServer::closeConn_, this, &users_[fd]));
    }
    epoller_->addFd(fd, EPOLLIN | connEvent_);
    setFdNonblock_(fd);
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
    epoller_->deleteFd(client->getFd());
    client->closeConn();
}

