/**
 * @file HeapTest.cpp
 * @author jack
 * @brief 
 * @version 0.1
 * @date 2022-08-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Heap.h"
#include <iostream>


int main(){
    Heap<int> h(64);
    h.push(15);
    h.push(2);
    h.push(16);
    h.push(4);

    while(!h.empty()){
        int i = 0;
        h.pop(i);
        std::cout << i << "\n"; 
    }

}