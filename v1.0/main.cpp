/**
 * @file main.cpp
 * @author jack
 * @brief  主函数
 * @version 0.1
 * @date 2022-08-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../net/WebServer.h"
#include "../config/config.h"

int main(){
    /* 读取参数 */
    Config config;
    char filename[] = "config.json";
    config.parseJson(filename);

    /* 初始化服务器 */
    WebServer server;
    server.init(config);

    /* 启动服务器 */
    server.start();
}