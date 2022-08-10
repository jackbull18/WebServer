/**
 * @file buffer.h
 * @author jack
 * @brief 自动增长缓冲区
 * @version 0.1
 * @date 2022-07-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef BUFFER
#define BUFFER
#include <vector>
#include <atomic>
#include <string.h>
#include <unistd.h>
#include <string>
#include <assert.h>
#include <sys/uio.h>
#include <errno.h>

class Buffer{
public:
    /* 构造函数和析构函数 */
    Buffer(const int initBuffSize = 1024):buffer_(initBuffSize),readPos_(0),writePos_(0){}
    ~Buffer() = default;

    /* 查询缓冲区信息 */
    size_t writableBytes() const{return buffer_.size() - writePos_;} // 可写的区域大小
    size_t readableBytes() const{return writePos_ - readPos_;} // 可读的区域大小
    size_t prependableBytes() const{return readPos_;} //已读区域的大小

    /* 返回当前缓冲区迭代器 */
    const char* peek() const{return beginPtr_() + readPos_;}
    const char* beginWriteConst() const{return beginPtr_() + writePos_;}
    char* beginWrite(){return beginPtr_() + writePos_;}
    
    /* 更新buff状态 */
    void hasWritten(size_t len){writePos_ += len;}

    /* 确保能够写入数据 */
    void ensureWriteable(size_t len);
    
    
    /* 封装read 和 write，与打开的文件描述符互动 */
    ssize_t readFd(int fd, int* saveErrno); // 分段读
    ssize_t writeFd(int fd, int* saveErrno); // 一次性写

    /* 加入buffer */
    void append(const char* str, size_t len);
    void append(const std::string& str);


    

    /* 清除buff */
    void clearBuffer(size_t len);
    void clearBufferUntil(const char* end);
    void clearBufferAll();
    std::string clearBufferAllToStr();

    

private:
    char* beginPtr_(){return &*buffer_.begin();};
    const char* beginPtr_() const{return &*buffer_.begin();};
    void makeSpace_(size_t len);

private:
    // 适用vector取代char数组管理缓冲区
    std::vector<char> buffer_;
    // 缓冲区当前读位置
    std::atomic<std::size_t> readPos_;
    // 缓冲区当前写位置
    std::atomic<std::size_t> writePos_;
};

#endif