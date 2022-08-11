/**
 * @file SQLConnPool.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "SQLConnPool.h"

SQLConnPool* SQLConnPool::instance(){
    static SQLConnPool pool;
    return &pool;
}

SQLConnPool::SQLConnPool(){
    
}

SQLConnPool::~SQLConnPool(){
    closeConnPool();
}

MYSQL* SQLConnPool::getConn(){
    MYSQL* mysql = nullptr;
    if(connQue_.empty()){
        return nullptr;
    }
    sem_wait(&semId_);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        mysql = connQue_.front();
        connQue_.pop();
    }
    return mysql;
}

void SQLConnPool::freeConn(MYSQL* mysql){
    assert(mysql);
    std::lock_guard<std::mutex> lock(mtx_);
    connQue_.push(mysql);
    sem_post(&semId_);
}

void SQLConnPool::closeConnPool(){
    std::lock_guard<std::mutex> lock(mtx_);
    while(!connQue_.empty()){
        auto sql = connQue_.front();
        connQue_.pop();
        mysql_close(sql);
    }
    mysql_library_end();
}

void SQLConnPool::init(const char* host, int port,
                           const char* user,const char* pwd, 
                           const char* dbName, int connSize)
{
    assert(connSize > 0);
    for(int i = 0; i < connSize; i++){
        MYSQL* mysql = nullptr;
        mysql = mysql_init(mysql);
        assert(mysql);
        mysql = mysql_real_connect(mysql, host, user, pwd, dbName, port,nullptr,0);
        assert(mysql);
        connQue_.push(mysql);
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0,MAX_CONN_);
}