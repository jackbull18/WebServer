/**
 * @file testConfig.cpp
 * @author 测试config
 * @brief 
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "config.h"


int main(){
    Config config;
    char filename[] = "./config/config.json";
    config.parseJson(filename);
    std::cout << config.port << " " << config.sqlUser;
}