/**
 * @file KBuffer.h
 * @author jack
 * @brief 大小有限制的buffer
 * @version 0.1
 * @date 2022-08-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <string.h>

class KBuffer{
public:
    int avail() const {return static_cast<int>(end() - cur_);}
    void append(const char* buff, size_t len){
        if(static_cast<size_t>(avail()) > len){
            memcpy(cur_, buff, len);
            cur_ += len;
        }
    }
    void bzero() {memset(data_, 0, sizeof(data_));}
    int length() const { return static_cast<int>(cur_ - data_); }
private:
    const char* end() const {return data_ + sizeof(data_);}
public:
    static const int BUFFER_SIZE = 4000*1000;
private:
    char data_[BUFFER_SIZE];
    char* cur_;
};