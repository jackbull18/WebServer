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


void HTTPConn::init(int sockFd, const sockaddr_in& addr){
    assert(sockFd > 0);
    userCount++;
    addr_ = addr;
    fd_ = sockFd;
    writeBuff_.clearBufferAll();
    readBuff_.clearBufferAll();
    isClose_ = false;
}

void HTTPConn::closeConn(){
    response_.unmapFile();
    if(isClose_ == false){
        isClose_ = true;
        userCount--;
        close(fd_);
    }
}

ssize_t HTTPConn::read(int* saveErrno){
    ssize_t len = -1;
    len = readBuff_.readFd(fd_, saveErrno);
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
    }while(toWriteBytes() > 10240);
    return len;
}

bool HTTPConn::process(){
    request_.init();
    if(readBuff_.readableBytes() <= 0){
        return false;
    }else if(request_.parse(readBuff_)){
        response_.init(srcDir, request_.getPath(),200);
    }else{
        response_.init(srcDir, request_.getPath(), 400);
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
    return true;
}

