/**
 * @file BlockQueue.h
 * @author your name (you@domain.com)
 * @brief 阻塞队列
 * @version 0.1
 * @date 2022-07-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

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
        while(queue_.size() >= capacity_){
            condProducer_.wait(lock);  
        }
        queue_.push_back(item);
        condConsumer_.notify_one();
    }
    bool pop(T& item){
        std::unique_lock<std::mutex> lock(mtx_);
        while(queue_.empty()){
            condConsumer_.wait(lock);
            if(isClosed_){
                return false;
            }
        }
        item = queue_.front();
        queue_.pop_front();
        condProducer_.notify_one();
        return true;
    }
    bool pop(T& item, int timeout){
        std::unique_lock<std::mutex> lock(mtx_);
        while(empty()){
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
    void clear(){
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.clear();
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
    bool full() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size() >= capacity_;
    }
    size_t capacity(){
        std::lock_guard<std::mutex> lock(mtx_);
        return capacity_;
    }
    size_t size(){
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    void flush(){
        condConsumer_.notify_one();
    }
    void close(){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.clear();
            isClosed_ = true;
        }
        condConsumer_.notify_all();
        condProducer_.notify_all();
    }

    explicit BlockQueue(size_t capacity = 1000)
        :capacity_(capacity),
         queue_()
    {
        assert(capacity > 0);
        isClosed_ = false;
    }
    ~BlockQueue(){
        close();
    }
};
