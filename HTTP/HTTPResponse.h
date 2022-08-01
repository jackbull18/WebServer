/**
 * @file HTTPResponse.h
 * @author jack
 * @brief HTTP响应，返回请求的文件，生成响应报文
 * @version 0.1
 * @date 2022-07-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once
#include <Buffer.h>
#include <string>


class HttpResponse{
public:
    /* 构造函数与析构函数 */
    HttpResponse();
    ~HttpResponse();

    /* 初始化函数 */
    void init(std::string path, std::string srcDir, int code);

    /* 生成响应 */
    void createResponse(Buffer& buffer);

private:
    void dealErrorCode_();
    void addStateLine_(Buffer& buffer);
    void addHeader_(Buffer& buffer);
    void addContent_(Buffer& buffer);

private:
    /* 请求资源信息 */
    std::string path_;  //http请求的文件的地址
    std::string srcDir_; //该请求资源所在的绝对地址

    /* 请求的状态信息 */
    int code_;
};