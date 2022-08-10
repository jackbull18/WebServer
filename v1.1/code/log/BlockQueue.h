/**
 * @file BlockQueue.h
 * @author jack
 * @brief 阻塞队列
 * @version 0.1
 * @date 2022-07-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef BLOCKQUEUE
#define BLOCKQUEUE
#include <queue>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <cassert>

template <typename T>
class BlockQueue
{
private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable condProducer_;
    std::condition_variable condConsumer_;
    bool isClosed_;
    size_t capacity_;

    
public:
    void push(const T &item){
        std::unique_lock<std::mutex> lock(mtx_);
        // 队列已满，push操作等待，while循环防止假通知
        while(queue_.size() >= capacity_){
            condProducer_.wait(lock);
        }
        queue_.push(item);
        condConsumer_.notify_one(); //通知消费者消费
    }
    /**
     * @brief 取出队列操作 
     * @param item 
     * @return true 
     * @return false 
     */
    bool pop(T& item){
        std::unique_lock<std::mutex> lock(mtx_);
        // 队列为空，pop操作等待，while循环防止假通知
        while(queue_.empty()){
            condConsumer_.wait(lock);
            if(isClosed_){
                return false;
            }
        }
        item = queue_.front();
        queue_.pop();
        condProducer_.notify_one(); // 通知生产者生产
        return true;
    }
    /**
     * @brief 带等待时间的pop操作，等待timeout秒后放弃pop
     * @param item 
     * @param timeout 
     * @return true 
     * @return false 
     */
    bool pop(T& item, int timeout){
        std::unique_lock<std::mutex> lock(mtx_);
        while(queue_.empty()){
            if(condConsumer_.wait_for(lock, std::chrono::seconds(timeout)) == std::cv_status::timeout){
                return false;
            };
            if(isClosed_){
                return false;
            }
        }
        item = std::move(queue_.front());
        queue_.pop_front();
        condProducer_.notify_one();
        return true;
    };
    /**
     * @brief 清空队列
     */
    void clear(){
        std::lock_guard<std::mutex> lock(mtx_);
        std::queue<T> empty;
        std::swap(queue_, empty);
    }
    /**
     * @brief 判断队列是否为空
     * @return true 
     * @return false 
     */
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
    /**
     * @brief 判断队列是否满
     * @return true 
     * @return false 
     */
    bool full() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size() >= capacity_;
    }
    /**
     * @brief 返回队列的容量
     * @return size_t 
     */
    size_t capacity(){
        std::lock_guard<std::mutex> lock(mtx_);
        return capacity_;
    }
    /**
     * @brief 返回队列当前元素的个数
     * @return size_t 
     */
    size_t size(){
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }
    /**
     * @brief 通知消费者消费
     */
    void flush(){
        condConsumer_.notify_one();
    }
    /**
     * @brief 关闭队列，清除队列内容，并通知所有等待状态
     */
    void close(){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            std::queue<T> empty;
            std::swap(queue_, empty);
            isClosed_ = true;
        }
        condConsumer_.notify_all();
        condProducer_.notify_all();
    }
    /**
     * @brief Construct a new Block Queue object
     * @param capacity 默认容量
     */
    explicit BlockQueue(size_t capacity = 1000)
        :queue_(),capacity_(capacity)
         
    {
        assert(capacity > 0);
        isClosed_ = false;
    }
    /**
     * @brief Destroy the Block Queue object 
     */
    ~BlockQueue(){
        close();
    }
};

#endif
