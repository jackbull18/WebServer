/**
 * @file HTTPRequest.cpp
 * @author your name (you@domain.com)
 * @brief HTTP解析类实现
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <algorithm>
#include <regex>

#include "HTTPRequest.h"


HttpRequest::HttpRequest(){
    init();
}

void HttpRequest::init(){
    method_ = "";
    path_ = "";
    version_ = "";
    body_ = "";

    headers_.clear();

    state_ = REQUEST_LINE;
}

bool HttpRequest::parse(Buffer& buff){
    if(buff.readableBytes() <= 0){
        return false;
    }
    const char CRLF[] = "\r\n"; //HTTP报文结尾格式
    //开始解析
    while(buff.readableBytes() > 0 && state_ != FINISH){
        auto lineEnd = std::search(buff.peek(), buff.beginWriteConst(),CRLF,CRLF+2);
        std::string line(buff.peek(), lineEnd);
        switch(state_){
            case REQUEST_LINE:
                if(!parseRequestLine_(line)){
                    return false;
                }
                break;
            case HEADERS:
                parseHeader_(line);
                if(buff.readableBytes() <= 2){
                    state_ = FINISH;
                }
                break;
            case BODY:
                parseBody_(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.beginWrite()){
            break;
        }
        buff.clearBufferUntil(lineEnd + 2);
    }
    return true;
}

std::string HttpRequest::getPath() const{
    return path_;
}

std::string HttpRequest::getMethod() const{
    return method_;
}

std::string HttpRequest::getVersion() const{
    return version_;
}

void HttpRequest::getHeaders(std::unordered_map<std::string, std::string>& headers) const{
    headers = headers_;
}

bool HttpRequest::parseRequestLine_(const std::string& line){
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,pattern)){
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    return false;
}

void HttpRequest::parseHeader_(const std::string& line){
    std::regex pattern("^([^ ]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,pattern)){
        headers_[subMatch[1]] = subMatch[2];
    }else{
        state_ = BODY;
    }
}

void HttpRequest::parseBody_(const std::string& line){
    body_ = line;
    state_ = FINISH;
}