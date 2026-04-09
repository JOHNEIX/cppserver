#include "net/Channel.h"
#include "net/EventLoop.h"
#include <sys/epoll.h>

Channel::Channel():loop_(nullptr),fd_(-1),events_(0),added_to_epoll_(false){}

Channel::Channel(EventLoop* loop,int fd):loop_(loop),fd_(fd),events_(0),added_to_epoll_(false){}

void Channel::enable_read(bool oneshot){

    events_=EPOLLIN ;
    if(oneshot) events_ |= EPOLLONESHOT ;
    loop_->update(this);
}

void Channel::handle_event(){
    if(read_cb_) read_cb_();//目前只有这个功能。
}