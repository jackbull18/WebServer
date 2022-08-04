/**
 * @file HeapTimer.h
 * @author jack
 * @brief 小顶堆实现的计时器
 * @version 0.1
 * @date 2022-08-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <functional>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <assert.h>

#include "Heap.h"


/* 一些类型别名的定义 */
typedef std::function<void()> TimeOutCallback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp; //timestamp 时间戳
/* ---------------- */

/* 计时器节点结构体 */
struct TimeNode{
    int id; //节点id
    TimeStamp expirationTime; //节点到期时间
    TimeOutCallback callback; //节点到期回调函数
    bool operator< (const TimeNode& t){return expirationTime < t.expirationTime;} //比较运算符重载
};
/* ---------------- */
class HeapTimer{
public:
    /* 析构和构造函数 */
    HeapTimer(int capacity = 64):heap_(capacity){}
    ~HeapTimer(){
        heap_.clear();
        ref_.clear();
    }
    /* 节点操作接口 */
    void addNode(int id , int timeout, const TimeOutCallback& callback);
    void adjustNode(int id, int newTimeout);
    void removeNode(); 

    /* 超时处理函数 */
    void tick();
    int getNextTick();
private:
    void upFilter_(size_t i);
    bool downFilter_(size_t index, size_t n);
    void swapNode_(size_t i, size_t j);
    void delNode_(size_t index);
private:
    std::vector<TimeNode> heap_;
    std::unordered_map<int, size_t> ref_;
};

void HeapTimer::addNode(int id , int timeout, const TimeOutCallback& callback){
    size_t index;
    if(ref_.count(id) == 0){
        /* 新的节点，尾部插入，并调整堆 */
        index = heap_.size();
        ref_[id] = index;
        heap_.push_back({id, Clock::now()+MS(timeout), callback});
        upFilter_(index);
    }else{
        /* 节点存在调整堆 */
        index = ref_[id];
        heap_[index].expirationTime = Clock::now() + MS(timeout);
        heap_[index].callback = callback;
        // 先下滤后上滤
        if(!downFilter_(index, heap_.size())){upFilter_(index);}
    }
}

void HeapTimer::adjustNode(int id, int newTimeOut){
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expirationTime = Clock::now() + MS(newTimeOut);
    downFilter_(ref_[id],heap_.size());
}

void HeapTimer::upFilter_(size_t i){
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while(j >= 0){
        if(heap_[j] < heap_[i]){break;}
        swapNode_(i, j);
        i = j;
        j = (i - 1)/2;
    }
}

bool HeapTimer::downFilter_(size_t index, size_t n){
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n){
        if(j + 1 < n && heap_[j+1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        swapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeapTimer::swapNode_(size_t i, size_t j){
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void HeapTimer::tick(){
    //清除超时节点
    if(heap_.empty()){
        return;
    }
    while(!heap_.empty()){
        TimeNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expirationTime - Clock::now()).count() > 0){
            break;
        }
        node.callback();
        removeNode();
    }
}

int HeapTimer::getNextTick(){
    tick();
    size_t res = -1;
    if(!heap_.empty()){
        res = std::chrono::duration_cast<MS>(heap_.front().expirationTime - Clock::now()).count();
        if(res < 0) {res = 0;}
    }
    return res;
}

void HeapTimer::removeNode(){
    assert(!heap_.empty());
    delNode_(0);
}

void HeapTimer::delNode_(size_t index){
    assert(!heap_.empty() && index >= 0 && index < heap_.size());

    size_t i = index;
    size_t n = heap_.size()-1;
    assert(i <= n);
    if(i < n){
        swapNode_(i, n);
        if(!downFilter_(i,n)){
            upFilter_(i);
        }
    }

    ref_.erase(heap_.back().id);
    heap_.pop_back();
}