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

#ifndef CONFIG
#define CONFIG
#include <iostream>
#include <fstream>
#include "../json/json.h"
//#include "AsyncLog.h"

class Config{
public:
    Config(){}
    ~Config(){}
    //void parseArg(int argc, char* argv[]);
    bool parseJson(const char* filename);

public:
    int port;
    int trigMode; 
    int timeoutMS; 
    bool openLinger; 
    int sqlPort; 
    std::string sqlUser; 
    std::string sqlPwd; 
    std::string dbName; 
    int connPoolNum; 
    int threadNum;
    bool openLog; 
    int logLevel; 
    int logQueSize;
};



#endif