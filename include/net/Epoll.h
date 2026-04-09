#pragma once
#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include<net/Channel.h>

class Channel;

class Epoll{
public:
    Epoll();
    ~Epoll();

    bool add_channel(Channel* channel, uint32_t events);  // EPOLL_CTL_ADD，首次注册
    bool mod_channel(Channel* channel, uint32_t events);  // EPOLL_CTL_MOD，更新已注册的事件（如重置 ONESHOT）
    bool del_channel(Channel* channel);                   // EPOLL_CTL_DEL，移除监听

    int wait(struct epoll_event* events, int max_events, int timeout = -1); // 等待事件，返回就绪数量

    bool is_valid() { return is_valid_; }       // 检查 epoll_fd 是否创建成功
    int get_fd() const { return epoll_fd_; }    // 返回 epoll 文件描述符

    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;
private:
    int epoll_fd_;   // epoll_create1 返回的文件描述符
    bool is_valid_;  // 标记 epoll 是否初始化成功

};


#endif