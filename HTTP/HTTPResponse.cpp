/**
 * @file HTTPResponse.cpp
 * @author jack
 * @brief  响应报文类实现
 * @version 0.1
 * @date 2022-07-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "HTTPResponse.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

HttpResponse::HttpResponse(){
    path_ = "";
    srcDir_ = "";
    code_ = -1;
}
HttpResponse::~HttpResponse(){
    unmapFile();
}

void HttpResponse::init(std::string path, std::string srcDir, int code){
    if(mmFile_) { unmapFile(); }
    path_ = path;
    srcDir_ = srcDir;
    code_ = code;
    mmFile_ = nullptr; 
    mmFileStat_ = { 0 };
}

void HttpResponse::createResponse(Buffer& buffer){
    /* 判断请求的资源文件 */
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;
    }
    else if(!(mmFileStat_.st_mode & S_IROTH)) {
        code_ = 403;
    }
    else if(code_ == -1) { 
        code_ = 200; 
    }
    dealErrorCode_();
    addStateLine_(buffer);
    addHeader_(buffer);
    addContent_(buffer);
}

void HttpResponse::addStateLine_(Buffer& buffer){
    std::string status;
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    }
    else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buffer.append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::addHeader_(Buffer& buffer){
    buffer.append("Connection: ");
    buffer.append("close\r\n");
    buffer.append("Content-type: " + getFileType_() + "\r\n");
}

void HttpResponse::addContent_(Buffer& buffer){
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
    if(srcFd < 0){
        errorContent_(buffer, "File Not Found!");
        return;
    }

    /* 将文件映射到内存提高文件的访问速度MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    int* mmRet = (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mmRet == -1){
        errorContent_(buffer, "File Not Found!");
        return;
    }

    mmFile_ = (char*)mmRet;
    close(srcFd);
    buffer.append("Content-length: " + std::to_string(mmFileStat_.st_size) + "\r\n\r\n");
}

void HttpResponse::errorContent_(Buffer& buffer, std::string message){
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buffer.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buffer.append(body);
}

std::string HttpResponse::getFileType_(){
    /* 判断文件类型 */
    std::string::size_type idx = path_.find_last_of('.');
    if(idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::unmapFile(){
    if(mmFile_){
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

char* HttpResponse::file() {
    return mmFile_;
}

size_t HttpResponse::fileLen() const {
    return mmFileStat_.st_size;
}

void HttpResponse::dealErrorCode_(){
    if(CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}