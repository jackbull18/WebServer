/**
 * @file Log.h
 * @author jack
 * @brief log类的外观类
 * @version 0.1
 * @date 2022-07-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */
class Log{
    
    
}

/* 定义宏，方便LOG的使用 */
#define LOG_BASE(level, format, ...) \
    do{\
        Log* log = Log::instance();\
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
