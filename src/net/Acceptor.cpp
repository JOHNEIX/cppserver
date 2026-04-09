#include "net/Acceptor.h"
#include "log/Logger.h"
#include <unistd.h>

Acceptor::Acceptor(EventLoop* loop, int port, Threadpool* pool)
    : loop_(loop),
      pool_(pool)
{
    //绑定+监听
    if (!listen_sock_.bind_listen(port)) {
        exit(1);
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

    // 创建客户端Channel，用 shared_ptr 管理生命周期
    auto client_ch = std::make_shared<Channel>(loop_, client_fd);
    // closed 标志防止多个线程任务重复关闭同一 fd
    auto closed = std::make_shared<std::atomic<bool>>(false);

    // 绑定回调，提交到线程池
    client_ch->set_read_callback([=]() {
        pool_->enqueue([=]() {
            if (closed->load()) return;

            char buffer[1024] = {0};
            ssize_t bytes_read = read(client_fd, buffer, 1024);

            if (bytes_read > 0) {
                // 回声回写
                send(client_fd, buffer, bytes_read, 0);
                // 重置 EPOLLONESHOT
                client_ch->enable_read(true);
            } else if (bytes_read == 0) {
                LOG_INFO("客户端 FD %d 断开连接", client_fd);
                closed->store(true);
                close(client_fd);
            } else {
                LOG_ERROR("读取 FD %d 数据失败", client_fd);
                closed->store(true);
                loop_->remove(client_ch.get());
                close(client_fd);
            }
        });
    });

    client_ch->enable_read(true);


}