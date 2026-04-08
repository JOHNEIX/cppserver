#include "net/Channel.h"
#include "net/EventLoop.h"
#include <sys/epoll.h>

Channel::Channel():loop_(nullptr),fd_(-1),events_(0),added_to_epoll_(false){}

Channel::Channel(EventLoop* loop,int fd):loop_(loop),fd_(fd),events_(0),added_to_epoll_(false){}

void Channel::enable_read(){
    events_=EPOLLIN | EPOLLONESHOT;
    loop_->update(this);
}

void Channel::handle_event(){
    if(read_cb_) read_cb_();
}