#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <cstdint>
#include <sys/epoll.h>
#include "net/Epoll.h"
#include "log/Logger.h"

class EventLoop;

class Channel{
public:
    Channel();
    Channel(EventLoop* loop,int fd);
    ~Channel()=default;

    void handle_event();
    void set_read_callback(std::function<void()> cb) { read_cb_ = std::move(cb);  }
    void enable_read();

    int fd() const{ return fd_;}
    uint32_t events() const {return events_;}

    bool is_added() const { return added_to_epoll_; }
    void set_added(bool added) { added_to_epoll_ = added; }

private:
    EventLoop* loop_;
    int fd_;
    uint32_t events_;               //监听的事件，比如EPOLLIN
    std::function<void()> read_cb_; //读回调
    bool added_to_epoll_; // 标记是否已添加到epoll，避免重复ADD
};


#endif

