#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include "net/Channel.h"
#include "net/Epoll.h"

class EventLoop{

public:
    EventLoop();
    ~EventLoop();

    void loop();

    void update(Channel* ch);
    void remove(Channel* ch);
    int fd() const{ return epoll_->get_fd();}
private:
    Epoll* epoll_;
    std::vector<Channel*> channels_;
    bool stop_;


};

#endif