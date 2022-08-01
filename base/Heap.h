/**
 * @file Heap.h
 * @author jack
 * @brief 小根堆
 * @version 0.1
 * @date 2022-08-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <vector>
template <typename T>
class Heap{
public:
    /* 构造、析构函数 */
    Heap(int capacity) : capacity_(capacity) {
        heap_.reserve(capacity);
    }
    ~Heap(){clear()};

    /* 数据查询接口 */
    int getCapacity() const{return capacity_;};
    bool full() const {return heap_.size() >= capacity_;};
    bool empty() const {return heap_.empty();};
    size_t size() const {return heap_.size();};

     /* 数据清除 */
    void clear(){heap_.clear();};

    /* 添加、弹出函数 */
    void pop(T& node);
    void push(T& node);

private:
    void adjustUp_(size_t i);
    void adjustDown_(size_t i);

private:
    std::vector<T> heap_;
    int capacity_;
};
template <typename T>
void Heap<T>::pop(T& node) {

}

template <typename T>
void Heap<T>::push(T& node) {

}
