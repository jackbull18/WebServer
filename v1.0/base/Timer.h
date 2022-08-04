/**
 * @file stdTimer.h
 * @author jack
 * @brief chrono库简单封装
 * @version 0.1
 * @date 2022-07-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <chrono>
#include <iostream>
#include <sys/time.h>

class StdTimer{
public:
    StdTimer(){
    }
    void init(){
        time_t timer = time(nullptr);
        struct tm *sysTime = localtime(&timer);
        struct tm t = *sysTime;
        snprintf(nowDate, sizeof(nowDate),"%04d_%02d_%02d",t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    };
    char* getNowDate(){
        return nowDate;
    }
    char* getNowTime(){
        struct timeval now = {0, 0};
        gettimeofday(&now, nullptr);
        time_t tSec = now.tv_sec;
        struct tm *sysTime = localtime(&tSec);
        struct tm t = *sysTime;
        snprintf(nowTime, sizeof(nowTime), "%02d:%02d:%02d.%06ld",t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        return nowTime;
    }
private:
    char nowDate[13];
    char nowTime[16];

};

