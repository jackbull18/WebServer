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
    ~Heap(){clear();};

    /* 数据查询接口 */
    int getCapacity() const{return capacity_;};
    bool full() const {return heap_.size() >= capacity_;};
    bool empty() const {return heap_.empty();};
    size_t size() const {return heap_.size();};

     /* 数据清除 */
    void clear(){heap_.clear();};

    /* 添加、弹出函数 */
    bool pop(T& node);
    void push(T&& node);

private:
    void adjustUp_(size_t index);
    void adjustDown_();

private:
    std::vector<T> heap_;
    int capacity_;
};
template <typename T>
void Heap<T>::push(T&& node) {
    heap_.push_back(node); //默认添加在堆底
    adjustUp_(heap_.size()-1); //堆该节点进行提升
}

template <typename T>
bool Heap<T>::pop(T& node) {
    if(!empty()){
        node = heap_.front();
        std::swap(heap_.front(), heap_.back());
        heap_.pop_back();
        adjustDown_();
        return true;
    }
    return false;
}

template <typename T>
void Heap<T>::adjustUp_(size_t index) {
    while(index > 0 && heap_[index] < heap_[index/2]){
        std::swap(heap_[index], heap_[index/2]);
        index /= 2;
    }
}
/**
 * @brief 将需要删除的元素下沉到向量的最后
 * 
 * @tparam T 
 * @param index 
 * @param t 
 */
template <typename T>
void Heap<T>::adjustDown_() {
    size_t i = 0;
    size_t j = i*2 + 1;
    size_t n = heap_.size();
    while(j < n){
        if(j + 1 < n && heap_[j+1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        std::swap(heap_[i], heap_[j]);
        i = j;
        j = i *2 + 1;
    }
}