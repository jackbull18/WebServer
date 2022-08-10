/**
 * @file Epoller.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-08-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Epoller.h"

inline Epoller::Epoller(int maxEvent):epollFd_(epoll_create(8)),events_(maxEvent){
    assert(epollFd_ >= 0 && events_.size() > 0);
}
inline Epoller::~Epoller(){
    close(epollFd_);
}

inline bool Epoller::addFd(int fd, uint32_t events){
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev);
}

inline bool Epoller::modfildFd(int fd, uint32_t events){
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

inline bool Epoller::deleteFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

/* 返回epoll_event信息 */
inline int Epoller::getEventFd(size_t i) const{ //返回已经注册的文件描述符
    assert(i > 0 && i < events_.size());
    return events_[i].data.fd;
} 
inline uint32_t Epoller::getEvents(size_t i) const{ //获取epoll事件
    assert(i > 0 && i < events_.size());
    return events_[i].events;
} 

/* epoll_wait */
inline int Epoller::wait(int timeoutMs){
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()),timeoutMs);
}