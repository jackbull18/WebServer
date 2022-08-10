/**
 * @file SQLConnPool.h
 * @author jack
 * @brief 数据库连接池
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef SQLCONNPOLL
#define SQLCONNPOLL

#include <queue>
#include <mutex>

#include <mysql/mysql.h>
#include <semaphore.h>
#include <assert.h>

class SQLConnPool{
public:
    /* 单例模式 */
    static SQLConnPool* instance(){
        static SQLConnPool pool;
        return &pool;
    }
    void init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);

    /* 数据库连接操作接口 */
    MYSQL* getConn();
    void freeConn(MYSQL* mysql);

    /* 连接池信息获取 */
    int getFreeConnCount(){
        std::lock_guard<std::mutex> lock(mtx_);
        return connQue_.size();
    }
    int getMaxConnCount()const{return MAX_CONN_;}

    /* 关闭数据库 */
    void closeConnPool();


private:
    /* 单例模式构造函数私有化 */
    SQLConnPool(){}
    ~SQLConnPool(){closeConnPool();}
private:
    /* 连接池信息 */
    int MAX_CONN_;

    /* 连接池容器 */
    std::queue<MYSQL*> connQue_;

    /* 连接池线程同步 */
    std::mutex mtx_;
    sem_t semId_;
};


#endif