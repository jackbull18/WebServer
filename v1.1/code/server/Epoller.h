/**
 * @file epoller.h
 * @author your name (you@domain.com)
 * @brief 封装epoll的操作
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef EPOLLER
#define EPOLLER

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>


class Epoller{
public:
    /* 构造函数和析构函数，封装了epoll_create函数 */
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    /* 操作epoll */
    bool addFd(int fd, uint32_t events); //添加epoll事件
    bool modfildFd(int fd, uint32_t events); // 修改epoll事件
    bool deleteFd(int fd); //删除epoll注册事件
public:
    /* 返回epoll_event信息 */
    int getEventFd(size_t i) const;
        
    uint32_t getEvents(size_t i) const;
    /* epoll_wait */
    int wait(int timeoutMs);
private:
    /* epoll 的文件描述符 */
    int epollFd_;
    /* 事件集合 */
    std::vector<struct epoll_event> events_; 
};
#endif