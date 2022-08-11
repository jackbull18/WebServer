/**
 * @file WebServer.h
 * @author jack
 * @brief 服务器
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef WEBSERVER
#define WEBSERVER


#include "../config/config.h"
#include "Epoller.h"
#include "../timer/HeapTimer.h"
#include "../pool/ThreadPool.h"
#include "../log/log.h"
#include "../pool/SQLConnPool.h"
#include "../pool/SQLConnRAII.h"
#include "../http/HTTPConn.h"



#include <memory>
#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



class WebServer{
public:
    /* 构造、析构和初始化函数 */
    WebServer(){}
    ~WebServer();
    void init(int port, int timeoutMS, bool optLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize, int trigMode);
    
    void init(Config &config);
    
    /* 服务器启动 */
    void start();
private:
    /* 服务器内部函数 */
    /* 初始化连接 */
    bool initSocket_();

    /* 初始化事件模式 */
    void initEventMode_(int trigMode);

    /* 初始化日志 */
    void initLog_(int logLevel, int logQueSize);

    /* 添加连接 */
    void addClient_(int fd, sockaddr_in addr);

    /* 处理连接 */
    /* 将处理放入线程池的函数 */
    void dealListen_();
    void dealRead_(HTTPConn* client);
    void dealWrite_(HTTPConn* client);
    /* 真正处理连接的函数 */
    void onRead_(HTTPConn* client);
    void onWrite_(HTTPConn* client);
    void onProcess_(HTTPConn* client);

    /* 关闭连接 */
    void closeConn_(HTTPConn* client);

    /* 辅助函数 */
    int setFdNonblock_(int fd);
    void sendError_(int fd, const char* info);
    void addSurvivalTime_(HTTPConn* client);


private:
    /* 服务器参数 */
    int port_;                                  /* 监听的端口 */
    int timeoutMS_;                             /* 连接保持时间 */
    bool isClose_;                              /* 服务器是否关闭 */
    int listenFd_;                              /* 监听文件描述符 */
    char* srcDir_;                              /* 资源地址 */
    bool openLinger_;                           /* 是否开启linger */
    int threadNum_;
    int sqlConnNum_;

    static const int MAX_FD = 65536;            /* 最大的连接数量 */

    uint32_t listenEvent_;                      /* 监听Epoll事件 */
    uint32_t connEvent_;                        /* 连接Epoll事件 */

    std::unique_ptr<HeapTimer> timer_;          /* 控制连接的定时器 */
    std::unique_ptr<ThreadPool> threadpool_;    /* 线程池 */
    std::unique_ptr<Epoller> epoller_;          /* epoll */
    std::unordered_map<int, HTTPConn> users_;   /* 连接文件描述符和连接 */
};

#endif