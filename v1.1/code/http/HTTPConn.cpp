/**
 * @file HTTPConn.cpp
 * @author jack
 * @brief HTTP连接类实现
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "HTTPConn.h"
#include <assert.h>
#include <unistd.h>

const char* HTTPConn::srcDir;
std::atomic<int> HTTPConn::userCount;
bool HTTPConn::isET;

HTTPConn::HTTPConn(){
        fd_ = -1;
        addr_ = { 0 };
        isClose_ = true;
    }
HTTPConn::~HTTPConn(){
    closeConn();
}

int HTTPConn::getFd() const{return fd_;}
int HTTPConn::getPort() const{return addr_.sin_port;}
const char* HTTPConn::getIP() const{return inet_ntoa(addr_.sin_addr);}
sockaddr_in HTTPConn::getAddr() const{return addr_;};
int HTTPConn::toWriteBytes() {return iov_[0].iov_len + iov_[1].iov_len;} 
bool HTTPConn::isKeepAlive()const{return request_.isKeepAlive();}

void HTTPConn::init(int sockFd, const sockaddr_in& addr){
    assert(sockFd > 0);
    userCount++;
    addr_ = addr;
    fd_ = sockFd;
    writeBuff_.clearBufferAll();
    readBuff_.clearBufferAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, getIP(), getPort(), (int)userCount);
}

void HTTPConn::closeConn(){
    response_.unmapFile();
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, getIP(), getPort(), (int)userCount);
    }
}

ssize_t HTTPConn::read(int* saveErrno){
    ssize_t len = -1;
    do{
        len = readBuff_.readFd(fd_, saveErrno);
        if(len < 0){
            break;
        }
    }while(isET);
    return len;
}

ssize_t HTTPConn::write(int* saveErrno){
    ssize_t len = -1;
    
    do{
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len == 0) break;
        else if(static_cast<size_t>(len) > iov_[0].iov_len){
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len){
                writeBuff_.clearBufferAll();
                iov_[0].iov_len = 0;
            }
        }else{
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.clearBuffer(len);
        }
    }while(isET || toWriteBytes() > 10240);
    return len;
}

bool HTTPConn::process(){
    request_.init();
    if(readBuff_.readableBytes() <= 0){
        return false;
    }else if(request_.parse(readBuff_)){
        LOG_DEBUG("%s", request_.getPath().c_str());
        response_.init(srcDir, request_.getPath(),200, request_.isKeepAlive());
    }else{
        response_.init(srcDir, request_.getPath(), 400, false);
    }

    response_.createResponse(writeBuff_);
    iov_[0].iov_base = const_cast<char*>(writeBuff_.peek());
    iov_[0].iov_len = writeBuff_.readableBytes();
    iovCnt_ = 1;

    if(response_.fileLen() > 0 && response_.file()){
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.fileLen() , iovCnt_, toWriteBytes());
    return true;
}

