/**
 * @file HTTPRequest.h
 * @author jack
 * @brief HTTP报文解析类，处理HTTP报文
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>
#include <unordered_map>

#include "Buffer.h"

class HTTPRequest{
public:
    /* 构造、析构及初始化函数 */
    HTTPRequest();
    ~HTTPRequest() = default;

    void init(); //多线程中将构造和初始化分开，能够加强异常安全

    /* 报文解析枚举类 */
     enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };
    
    /* 获取请求行解析的接口 */
    std::string getPath() const;
    std::string getMethod() const;
    std::string getVersion() const;
    void getHeaders(std::unordered_map<std::string, std::string>& headers) const;
    

    /* 报文解析接口 */
    bool parse(Buffer& buff);

private:
    /* 解析报文 */
    bool parseRequestLine_(const std::string& line);
    void parseHeader_(const std::string& line);
    void parseBody_(const std::string& line);



private:
    /* 解析状态成员变量 */
    PARSE_STATE state_; // 使用成员变量的形式表示当前的解析状态
    /* 成员变量存放报文解析的结果 */
    //请求行
    std::string method_;    // 请求方法
    std::string path_;      //请求文件地址
    std::string version_;   //HTTP版本

    //首部行
    std::unordered_map<std::string, std::string> headers_;

    //报文体
    std::string body_; 

};