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
#include <sys/epoll.h>
#include <assert.h>
#include <unistd.h>
#include <vector>


class Epoller{
public:
    /* 构造函数和析构函数，封装了epoll_create函数 */
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    /* 操作epoll */
    bool AddFd(int fd, uint32_t events); //添加epoll事件
    bool ModfildFd(int fd, uint32_t events); // 修改epoll事件
    bool DeleteFd(int fd); //删除epoll注册事件
public:
    /* 返回epoll_event信息 */
    int getEventFd(size_t i) const{ //返回已经注册的文件描述符
        assert(i > 0 && i < events_.size());
        return events_[i].data.fd;
    } 
    uint32_t getEvents(size_t i) const{ //获取epoll事件
        assert(i > 0 && i < events_.size());
        return events_[i].events;
    } 

    /* epoll_wait */
    int wait(int timeoutMs){
        return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()),timeoutMs);
    }

 
private:
    /* epoll 的文件描述符 */
    int epollFd_;
    /* 事件集合 */
    std::vector<struct epoll_event> events_; 
};

Epoller::Epoller(int maxEvent = 1024):epollFd_(epoll_create(8)),events_(maxEvent){
    assert(epollFd_ >= 0 && events_.size() > 0);
}
Epoller::~Epoller(){
    close(epollFd_);
}

bool Epoller::AddFd(int fd, uint32_t events){
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModfildFd(int fd, uint32_t events){
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DeleteFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}