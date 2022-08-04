/**
 * @file buffer.cpp
 * @author jack
 * @brief buffer类的实现
 * @version 0.1
 * @date 2022-07-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Buffer.h"
#include <assert.h>
#include <unistd.h>
#include <sys/uio.h>
#include <errno.h>

void Buffer::ensureWriteable(size_t len){
    if(writableBytes() < len){
        makeSpace_(len);
    }
    assert(writableBytes() >= len);
}

ssize_t Buffer::readFd(int fd, int* saveErrno){
    // 分段读，确保将数据全部读入
    char extra_buff[65535];
    struct iovec iov[2];
    const size_t writable = writableBytes(); 

    iov[0].iov_base = beginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = extra_buff;
    iov[1].iov_len = sizeof(extra_buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0){
        *saveErrno = errno;
    }else if(static_cast<size_t>(len) <= writable){
        hasWritten(len);  //写入数据时，需要手动设置写入指针
    }else{
        writePos_ = buffer_.size();
        append(extra_buff, len - writable);
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno){
    //一次性写完
    const size_t readSize = readableBytes();
    const ssize_t len = write(fd, peek(), readSize);
    if(len < 0){
        *saveErrno = errno;
        return len;
    }
    readPos_ += len;
    return len;
}

void Buffer::append(const char* str, size_t len){
    assert(str);
    ensureWriteable(len);
    std::copy(str,str+len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string& str){
    append(str.data(), str.length());
}

void Buffer::clearBuffer(size_t len){
    assert(len <= readableBytes());
    readPos_ += len;
}

void Buffer::clearBufferUntil(const char* end){
    assert(peek() <= end);
    clearBuffer(end - peek());
}

void Buffer::clearBufferAll(){
    bzero(&buffer_[0],buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::clearBufferAllToStr(){
    std::string str(peek(), readableBytes());
    clearBufferAll();
    return str;
}

void Buffer::makeSpace_(size_t len){
    if(writableBytes() + prependableBytes() < len){
        buffer_.resize(writePos_ + len + 1);
    }else{
        const size_t readable = readableBytes();
        std::copy(beginPtr_()+ readPos_, beginPtr_()+ writePos_,beginPtr_());
        readPos_ = 0;
        writePos_ = readable;
        assert(readable == readableBytes());
    }
}