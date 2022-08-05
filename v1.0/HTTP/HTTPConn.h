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

#pragma once

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "../base/Buffer.h"

class HTTPConn{
public:
    /* 构造、析构、初始化和关闭函数 */
    HTTPConn(){
        fd_ = -1;
        addr_ = { 0 };
        isClose_ = true;
    }
    ~HTTPConn(){
        closeConn();
    }
    void init(int sockFd, const sockaddr_in& addr);
    void  closeConn();

    /* 读写接口 */
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);

    /* 信息获取接口 */
    int getFd() const{return fd_;}
    int getPort() const{return addr_.sin_port;}
    const char* getIP() const{return inet_ntoa(addr_.sin_addr);}
    sockaddr_in getAddr() const{return addr_;};
    int toWriteBytes() {return iov_[0].iov_len + iov_[1].iov_len;} 
    bool isKeepAlive()const{return request_.isKeepAlive();}

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