/**
 * @file config.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "config.h"

inline bool Config::parseJson(const char* filename){
    std::ifstream ifs;
    ifs.open(filename);
    if(!ifs.is_open()){
        //LOG_ERROR("open json file failed!");
        return false;
    }
    bool res;
    Json::CharReaderBuilder readerBuilder;
    std::string err_json;
    Json::Value json_obj;
    try{
        bool ret = Json::parseFromStream(readerBuilder, ifs, &json_obj, &err_json);
        if(!ret){
            //LOG_ERROR("invaild json file : %s", filename);
            return false;
        }
    } catch(std::exception &e){
        //LOG_ERROR("exception while parse json file:%s %s", filename, e.what());
        return false;
    }
    port = json_obj["port"].asInt();
    trigMode = json_obj["trigMode"].asInt();
    timeoutMS = json_obj["timeoutMS"].asInt();
    openLinger =json_obj["optLinger"].asInt();
    sqlPort = json_obj["sqlPort"].asInt();
    sqlUser = json_obj["sqlUser"].asString();
    sqlPwd = json_obj["sqlPwd"].asString();
    dbName = json_obj["dbName"].asString();
    connPoolNum = json_obj["connPoolNum"].asInt();
    threadNum = json_obj["threadNum"].asInt();
    openLog = json_obj["openLog"].asInt();
    logLevel = json_obj["logLevel"].asInt();
    logQueSize = json_obj["logQueSize"].asInt();
    return true;
}
