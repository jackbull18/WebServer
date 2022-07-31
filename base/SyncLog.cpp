/**
 * @file syncLog.cpp
 * @author your name (you@domain.com)
 * @brief 同步日志类实现
 * @version 0.1
 * @date 2022-07-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "SyncLog.h"
#include "Timer.h"
#include <assert.h>
#include <chrono>
#include <stdio.h>
#include <stdarg.h>


SyncLog::SyncLog(){
    fp_ = nullptr;
    lineCount_ = 0;
}

SyncLog::~SyncLog(){
    if(fp_){
        std::lock_guard<std::mutex> lock(mtx_);
        flush();
        fclose(fp_);
    }
}

SyncLog* SyncLog::instance(){
    static SyncLog Logger;
    return &Logger;
}

void SyncLog::init(int level, const char* path, const char* suffix){
    /* 信息同步 */
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;

    timer_.init();

    /* 打开日志文件，如果不存在则创建文件,日志保存文件不存在则创建文件夹 */
    // 构建文件名
    char logFileName[LOG_NAME_LEN] = {0};
    snprintf(logFileName, LOG_NAME_LEN -1, "%s/%s%s",path_,timer_.getNowDate(),suffix_);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        buffer_.clearBufferAll();
        if(fp_){
            flush();
            fclose(fp_);
        }
        fp_ = fopen(logFileName, "a"); //已添加的方式打开文件，日志会填充到文件末尾
        if(fp_ == nullptr){
            mkdir(path_, 0777);
            fp_ = fopen(logFileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void SyncLog::write(int level, const char* format,...){
    // 日志记录满，重新创建日志文件
    if(lineCount_ && (lineCount_ % MAX_LINES == 0)){
        createNewLogFile_();
    }
    //记录日志
    {   
        std::lock_guard<std::mutex> lock(mtx_);
        lineCount_++;
        // 添加时间
        int n = snprintf(buffer_.beginWrite(),128,"%s %s",timer_.getNowDate(),timer_.getNowTime());
        buffer_.hasWritten(n);
        // 添加日志等级
        appendLogLevelTitle_(level);

        //添加额外参数
        va_list vaList;
        va_start(vaList, format);
        int m = vsnprintf(buffer_.beginWrite(),buffer_.writableBytes(),format,vaList);
        va_end(vaList);
        buffer_.hasWritten(m);

        //添加结尾
        buffer_.append("\n\0",2);

        //将日志写入文件流中
        fputs(buffer_.peek(),fp_);
        buffer_.clearBufferAll();
    }
}

void SyncLog::createNewLogFile_(){
     std::unique_lock<std::mutex> lock(mtx_);
     lock.unlock();

     char newFileName[LOG_NAME_LEN];
     snprintf(newFileName, LOG_NAME_LEN-72, "%s/%s-%d%s",path_,timer_.getNowDate(),(lineCount_ / MAX_LINES),suffix_);

     lock.lock();
     flush();
     fclose(fp_);
     fp_ = fopen(newFileName, "a");
     assert(fp_ != nullptr);


}

void SyncLog::appendLogLevelTitle_(int level){
    switch(level){
        case 0:
            buffer_.append("[debug]: ",9);
            break;
        case 1:
            buffer_.append("[info] : ",9);
            break;
        case 2:
            buffer_.append("[warn] : ",9);
            break;
        case 3:
            buffer_.append("[error]: ",9);
            break;
        default:
            buffer_.append("[info] : ",9);
            break;
    }
}

int SyncLog::getLevel(){
    std::lock_guard<std::mutex> lock(mtx_);
    return level_;
}

void SyncLog::setLevel(int level){
    std::lock_guard<std::mutex> lock(mtx_);
    level_ = level;
}
