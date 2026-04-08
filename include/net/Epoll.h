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

    bool add_channel(Channel* channel, uint32_t events);
    bool mod_channel(Channel* channel, uint32_t events);
    bool del_channel(Channel* channel);    

    int wait(struct epoll_event* events, int max_events, int timeout = -1);

    bool is_valid(){return is_valid_;}
    int get_fd() const { return epoll_fd_; }
  

    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;
private:
    int epoll_fd_;
    bool is_valid_;

};


#endif