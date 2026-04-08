#include "net/Acceptor.h"
#include "log/Logger.h"
#include <unistd.h>
#include <sys/epoll.h>

Acceptor::Acceptor(EventLoop* loop, int port, Threadpool* pool)
    : loop_(loop),
      pool_(pool)
{
    //绑定+监听
    if (!listen_sock_.bind_listen(port)) {
        LOG_ERROR("端口绑定失败，服务器退出！");
        exit(1); // 直接终止进程
    }
    // 用监听Socket的fd创建Channel
    accept_channel_ = Channel(loop, listen_sock_.get_fd());

    // 绑定回调：读事件触发时，调用handle_accept
    accept_channel_.set_read_callback([this]() {
        this->handle_accept();
    });

    // 开启监听
    accept_channel_.enable_read();
    LOG_INFO("Acceptor 初始化完成，监听端口: %d", port);
}


void Acceptor::handle_accept() {

    int client_fd = listen_sock_.accept();
    if (client_fd < 0) {
        LOG_ERROR("接受连接失败");
        return;
    }

    LOG_INFO("新客户端连接,FD %d", client_fd);

    // 创建客户端Channel
    Channel* client_ch = new Channel(loop_, client_fd);
    int epoll_fd = loop_->fd();

    // 绑定回调，提交到线程池
    client_ch->set_read_callback([=]() {
        pool_->enqueue([=]() {
            char buffer[1024] = {0};
            ssize_t bytes_read = read(client_fd, buffer, 1024);

            if (bytes_read > 0) {
                // 回声回写
                send(client_fd, buffer, bytes_read, 0);

                // 重置 EPOLLONESHOT
                epoll_event ev;
                ev.events = EPOLLIN | EPOLLONESHOT;
                ev.data.ptr = client_ch;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
            } else {
                // 关闭连接
                close(client_fd);
                delete client_ch;
            }
        });
    });

    client_ch->enable_read();
}