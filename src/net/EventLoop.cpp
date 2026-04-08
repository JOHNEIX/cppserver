#include "net/EventLoop.h"
#include "log/Logger.h"
#include <string>
#include <unistd.h>

EventLoop::EventLoop():epoll_(new Epoll()),stop_(false){

}

EventLoop::~EventLoop(){
    delete epoll_;
}

void EventLoop::loop(){
    LOG_INFO("EventLoop 循环启动");
    while(!stop_){
        epoll_event events[1024];
        int nfds=epoll_->wait(events,1024);
        if (nfds < 0) {
            LOG_ERROR("epoll_wait 失败, errno=%d", errno);
            continue;
        }
        for(int i=0;i<nfds;i++){
            Channel* ch=static_cast<Channel*>(events[i].data.ptr); 
            ch->handle_event();
        }
    }

}

void EventLoop::update(Channel* ch){
    if (!ch) return;
    epoll_->add_channel(ch,ch->events());
}

void EventLoop::remove(Channel* ch){
    epoll_->del_channel(ch);
}