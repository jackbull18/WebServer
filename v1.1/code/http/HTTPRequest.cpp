/**
 * @file HTTPRequest.cpp
 * @author jack
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

const std::unordered_set<std::string> HTTPRequest::DEFAULT_HTML{
            "/index", 
            "/register", 
            "/login",
            "/welcome", 
            "/video", 
            "/picture", };

const std::unordered_map<std::string, int> HTTPRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, 
            {"/login.html", 1},  };


HTTPRequest::HTTPRequest(){
    init();
}

void HTTPRequest::init(){
    method_ = "";
    path_ = "";
    version_ = "";
    body_ = "";

    headers_.clear();
    post_.clear();

    state_ = REQUEST_LINE;
}

bool HTTPRequest::parse(Buffer& buff){
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
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

std::string HTTPRequest::getPath() const{
    return path_;
}

std::string HTTPRequest::getMethod() const{
    return method_;
}

std::string HTTPRequest::getVersion() const{
    return version_;
}

void HTTPRequest::getHeaders(std::unordered_map<std::string, std::string>& headers) const{
    headers = headers_;
}

bool HTTPRequest::isKeepAlive()const{
    if(headers_.count("Connection") == 1){
        return headers_.find("Connection")->second == "Keep-Alive" && version_ == "1.1";
    }
    return false;
}

bool HTTPRequest::parseRequestLine_(const std::string& line){
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,pattern)){
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("Parse requestLine error!");
    return false;
}

void HTTPRequest::parseHeader_(const std::string& line){
    std::regex pattern("^([^ ]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,pattern)){
        headers_[subMatch[1]] = subMatch[2];
    }else{
        state_ = BODY;
    }
}

void HTTPRequest::parseBody_(const std::string& line){
    body_ = line;
    if(method_ == "POST"){
        parsePost_();
    }
    state_ = FINISH;
    LOG_DEBUG("Body: %s, len:%d", line.c_str(), line.size());
}

void HTTPRequest::parsePost_(){
    if(headers_["Content-Type"] == "application/x-www-from-urlencoded"){
        parseFromUrlEncoded_();
        if(DEFAULT_HTML_TAG.count(path_)){
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("HTML tag:%d", tag);
            if(tag == 0 || tag == 1){
                bool isLogin = (tag == 1);
                if(userVerify_(post_["username"],post_["password"], isLogin)){
                    path_ = "/welcome.html";
                }else{
                    path_ = "/error.html";
                }
            }
        }
    }
}

bool HTTPRequest::userVerify_(const std::string &name, const std::string &pwd, bool isLogin){
    if(name == "" || pwd == ""){
        return false;
    }
    LOG_INFO("Verify name:%s ,pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SQLConnRAII(&sql, SQLConnPool::instance()); //创建临时对象，释放时自动放回连接
    assert(sql);

    bool flag = false;
    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD* fields = nullptr;
    MYSQL_RES * res = nullptr;

    if(!isLogin){
        flag = true;
    }

    /* 组装SQL语句，查询用户名及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("MYSQL: %s", order);

    if(mysql_query(sql, order)){
        mysql_free_result(res);
        LOG_ERROR("MYSQL query error!");
        return false;
    }

    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    /* 数据库查询结果分析 */
    while(MYSQL_ROW row = mysql_fetch_row(res)){
        LOG_DEBUG("MTSQL ROW: %s %s", row[0], row[1]);
        std::string query_pwd(row[1]);
        if(isLogin){
            if(pwd == query_pwd){
                flag = true;
            }else{
                flag = false;
                LOG_ERROR("Password error!");
            }
        }else{
            flag = false;
            LOG_DEBUG("user name used!");
        }
    }

    mysql_free_result(res);
    if(!isLogin && flag == true){
        LOG_DEBUG("regirster a new user!");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s', '%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG("MYSQL: %s",order);
        if(mysql_query(sql,order)){
            LOG_DEBUG("MYSQL insert error!");
            flag = false;
        }
        flag = true;
    }
    LOG_INFO("User verify success!");
    return flag;
}


void HTTPRequest::parseFromUrlEncoded_() {
    if(body_.size() == 0) { return; }

    std::string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
        case '=':
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '%':
            num = converHex_(body_[i + 1]) * 16 + converHex_(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

int HTTPRequest::converHex_(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}