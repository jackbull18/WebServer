/**
 * @file ThreadPool.h
 * @author jack
 * @brief  单事件循环线程池
 * @version 0.1
 * @date 2022-07-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

#include <assert.h>

class ThreadPool{
public:
    explicit ThreadPool(int threadnums = 8):pool_(std::make_shared<pool>()){
        assert(threadnums > 0);
        for(int i = 0; i < threadnums; i++){
            std::thread([pool = pool_]{
                std::unique_lock<std::mutex> lock(pool->mtx);
                // 事件循环
                while(true){
                    if(!pool->tasks.empty()){
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        lock.unlock();
                        task();
                        lock.lock();
                    }else if(pool->isClosed){
                        break;
                    }else{
                        pool->cond.wait(lock);
                    }
                }
            }).detach();
        }
    }
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool(){
        if(static_cast<bool>(pool_)){
            pool_->isClosed = true;
            pool_->cond.notify_all();
        }  
    }

    template<typename F>
    void appendTask(F&& task){
        {
            std::lock_guard<std::mutex> lock(mtx_);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }
private:
    struct pool{
        std::atomic<bool> isClosed;
        std::mutex mtx;
        std::condition_variable cond;
        std::queue<std::function<void()>> tasks;  
    };
    std::shared_ptr<pool> pool_;
};