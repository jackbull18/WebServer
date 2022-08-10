/**
 * @file Log.h
 * @author jack
 * @brief 异步日志的另一种实现
 * @version 0.1
 * @date 2022-08-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "KBuffer.h"
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <vector>
#include <assert.h>

class Log{
public:
    /* 构造和析构函数 */
    Log();
    ~Log();

    /* log接口 */
    void append(const char* logline, int len);


    /* 日志器控制接口 */
    void start(){
        isRunning_ = true;
    }
    void stop(){
        isRunning_ = false;
        cond_.notify_all();
        writeThread_.join();
    }
private:
    void writeThreadFunc_();

private:

    typedef std::vector<std::unique_ptr<KBuffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    /* 与buffer相关的成员 */
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;

    /* 线程安全的成员变量 */
    std::mutex mtx_;
    std::condition_variable cond_;
    
    /* 日志器设置成员变量 */
    std::atomic<bool> isRunning_;
    const int flushTimeout_;

    /* 写日志线程 */
    std::thread writeThread_;

};

void Log::append(const char* logline, int len){
    std::lock_guard<std::mutex> locker(mtx_);
    if(currentBuffer_->avail() > len){
        currentBuffer_->append(logline,len);
    }else{
        buffers_.push_back(std::move(currentBuffer_));

        if(nextBuffer_){
            currentBuffer_ = std::move(nextBuffer_);
        }else{
            currentBuffer_.reset(new KBuffer);
        }
        currentBuffer_->append(logline,len);
        cond_.notify_one();
    }
}

void Log::writeThreadFunc_(){
    assert(isRunning_);

    /* 创建两块buffer用来替换 */
    BufferPtr newBuffer1(new KBuffer);
    BufferPtr newBuffer2(new KBuffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    /* 创建一个写端的buffer数组 */
    BufferVector writeBuffers_;
    writeBuffers_.reserve(16);

    while (isRunning_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(writeBuffers_.empty());

        {
            std::lock_guard<std::mutex> lock(mtx_);
            if(buffers_.empty()){
                cond_.wait_for(mtx_, std::chrono::duration<int>::seconds);
            }
        }
    }
    


}