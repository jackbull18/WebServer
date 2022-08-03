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
#include "Buffer.h"
#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap


class HTTPResponse{
public:
    /* 构造函数与析构函数 */
    HTTPResponse();
    ~HTTPResponse();

    /* 初始化函数 */
    void init(std::string path, std::string srcDir, int code);

    /* 生成响应 */
    void createResponse(Buffer& buffer);

    char* file();
    size_t fileLen() const;

    void unmapFile();

private:
    void dealErrorCode_();
    void addStateLine_(Buffer& buffer);
    void addHeader_(Buffer& buffer);
    void addContent_(Buffer& buffer);
    void errorContent_(Buffer& buffer, std::string message);

    std::string getFileType_();
    

private:
    /* 请求资源信息 */
    std::string path_;  //http请求的文件的地址
    std::string srcDir_; //该请求资源所在的绝对地址

    /* 请求的状态信息 */
    int code_;

    char* mmFile_;
    struct stat mmFileStat_;

    /* 一些组成报文的信息 */
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};