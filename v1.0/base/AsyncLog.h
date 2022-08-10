/**
 * @file AsyncLog.h
 * @author jack
 * @brief 异步日志类
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef ASYNCLOG
#define ASYNCLOG
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <mutex>
#include <memory>
#include <thread>
#include <assert.h>
#include <chrono>
#include <stdio.h>
#include <stdarg.h>

#include "Buffer.h"
#include "Timer.h"
#include "BlockQueue.h"

class AsyncLog{
public:
    /* 初始化函数 */
    void init(int level, const char* path, const char* suffix,int queSize);

    /* 懒汉单例模式 */
    static AsyncLog* instance();
    static void flushLogThread();
    /* 写日志接口 */
    void write(int level, const char* format,...);
    void flush(){fflush(fp_);}

    /* 日志等级相关函数 */
    int getLevel();
    void setLevel(int level);

    /* 判断日志器是否打开 */
    bool isOpen() const {return isOpen_;} 
private:
    /* 日志一般为单例模式，将构造函数隐藏 */
    AsyncLog();
    ~AsyncLog();

    /* 日志记录辅助函数 */
    void appendLogLevelTitle_(int level);
    void createNewLogFile_();
    void asyncWrite_();
    


private:
    /* 日志设置 */
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 500000;

    /* 日志写入缓存 */
    Buffer buffer_;

    /* 文件流 */
    FILE* fp_;

    /* 日志是否打开 */
    bool isOpen_;

    /* 日志器等级 */
    int level_;

    /* 日志文件信息 */
    const char *path_;
    const char *suffix_;

    /* 日志行数 */
    int lineCount_;

    /* 线程安全 */
    std::mutex mtx_;

    /* 获取时间 */
    StdTimer timer_;

    /* 阻塞队列实现日志异步 */
    std::unique_ptr<BlockQueue<std::string>> deque_;

    /* 写日志线程 */
    std::unique_ptr<std::thread> writeThread_;

};

/* 定义宏，方便LOG的使用 */
#define LOG_BASE(level, format, ...) \
    do{\
        AsyncLog* log = AsyncLog::instance();\
        if(log->isOpen() && log->getLevel() <= level){\
            log->write(level, format, ##__VA_ARGS__);\
            log->flush();\
        }\
    }while(0);

/* 不同等级的LOG */
#define LOG_DEBUG(format, ...) do{LOG_BASE(0,format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do{LOG_BASE(1,format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do{LOG_BASE(2,format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do{LOG_BASE(3,format, ##__VA_ARGS__)} while(0);

#endif