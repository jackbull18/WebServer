/**
 * @file HTTPConn.h
 * @author jack
 * @brief HTTP连接类
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef HTTPCONN
#define HTTPCONN

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "../buffer/Buffer.h"

class HTTPConn{
public:
    /* 构造、析构、初始化和关闭函数 */
    HTTPConn();
    ~HTTPConn();
    void init(int sockFd, const sockaddr_in& addr);
    void  closeConn();

    /* 读写接口 */
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);

    /* 信息获取接口 */
    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;
    int toWriteBytes();
    bool isKeepAlive()const;

    /* 连接处理接口 */
    bool process();

public:
    static const char* srcDir;
    static std::atomic<int> userCount;
    static bool isET;
private:
    /* 与连接相关的信息 */
    int fd_; //代表这个连接的文件描述符
    struct  sockaddr_in addr_;

    bool isClose_;
    
    int iovCnt_;
    struct iovec iov_[2];
    
    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HTTPRequest request_;
    HTTPResponse response_;

};

#endif