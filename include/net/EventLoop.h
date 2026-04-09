#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <vector>
#include "net/Channel.h"
#include "net/Epoll.h"

class EventLoop{

public:
    EventLoop();
    ~EventLoop();

    void loop();               // 启动事件循环，阻塞直到 stop_ 为 true
    void update(Channel* ch);  // 将 Channel 注册或更新到 epoll（ADD/MOD）
    void remove(Channel* ch);  // 从 epoll 中移除 Channel

private:
    Epoll* epoll_;                      // 封装了 epoll_fd 的操作
    std::vector<Channel*> channels_;    // 暂未使用，预留用于管理所有活跃 Channel
    bool stop_;                         // 控制 loop() 退出


};

#endif