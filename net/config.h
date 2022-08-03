/**
 * @file config.h
 * @author Jack
 * @brief 设置类
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

class Config{
public:
    Config();
    ~Config();

    void parse_arg(int argc, char* argv[]);
public:
    int portF;
    int trigMode; 
    int timeoutMS; 
    bool OptLinger; 
    int sqlPort; 
    const char* sqlUser; 
    const  char* sqlPwd; 
    const char* dbName; 
    int connPoolNum; 
    int threadNum;
    bool openLog; 
    int logLevel; 
    int logQueSize;
};