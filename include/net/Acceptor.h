#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "thread/ThreadPool.h"
#include <functional>

// 专门封装：监听fd + 接受新连接
class Acceptor {
private:
    EventLoop* loop_;
    Socket listen_sock_;          // 监听socket
    Channel accept_channel_;    // 监听fd的channel

    // 线程池（你的线程池）
    Threadpool* pool_;

    // 内部：真正处理accept的函数（封装在类内，无全局函数）
    void handle_accept();

public:
    // 构造：绑定反应堆 + 端口 + 线程池
    Acceptor(EventLoop* loop, int port, Threadpool* pool);
};

#endif