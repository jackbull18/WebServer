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

#ifndef HEAPTIMER
#define HEAPTIMER

#include <functional>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <assert.h>


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
    HeapTimer(int capacity = 64);
    ~HeapTimer();
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
#endif