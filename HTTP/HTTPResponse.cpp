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

HttpResponse::HttpResponse(){
    path_ = "";
    srcDir_ = "";
    code_ = -1;
}
HttpResponse::~HttpResponse(){

}

void HttpResponse::init(std::string path, std::string srcDir, int code){
    path_ = path;
    srcDir_ = srcDir;
    code_ = code;
}

void HttpResponse::createResponse(Buffer& buffer){
    dealErrorCode_();
    addStateLine_(buffer);
    addHeader_(buffer);
    addContent_(buffer);
}