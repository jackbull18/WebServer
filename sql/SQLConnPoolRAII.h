/**
 * @file SQLConnPoolRAII.h
 * @author jack
 * @brief 数据连接池RAII包装类
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "SQLConnPool.h"

class SQLConnRAII{
public:
    SQLConnRAII(MYSQL** sql, SQLConnPool *connpool) {
        assert(connpool);
        *sql = connPool_->getConn();
        sql_ = *sql;
        connPool_ = connpool;
    }
    
    ~SQLConnRAII() {
        if(sql_) { connPool_->freeConn(sql_); }
    }

private:
    MYSQL *sql_;
    SQLConnPool* connPool_;
};