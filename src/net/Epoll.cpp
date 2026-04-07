#include "net/Epoll.h"
#include <unistd.h>
#include "log/Logger.h"

Epoll::Epoll():epoll_fd_(-1),is_valid_(false){
    epoll_fd_=epoll_create1(0);
    if(epoll_fd_==-1){
        LOG_ERROR("创建epoll实例失败");
        is_valid_=false;
        return;
    }   
    is_valid_=true;
    LOG_INFO("epoll创建成功, FD %d", epoll_fd_);
}

Epoll::~Epoll(){
    if(is_valid_){
        close(epoll_fd_);
        LOG_INFO("epoll关闭, FD %d", epoll_fd_);
    }
}
//增加，主要用于新链接和新消息
bool Epoll::add_fd(int fd, uint32_t events) {
    if (!is_valid_) return false;
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOG_ERROR("epoll添加fd=%d失败", fd);
        return false;
    }
    return true;
}
//修改，主要是返回客户端
bool Epoll::mod_fd(int fd, uint32_t events) {
    if (!is_valid_) return false;
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        LOG_ERROR("epoll修改fd=%d失败", fd);
        return false;
    }
    return true;
}

// 删除 fd，暂时没用
bool Epoll::del_fd(int fd) {
    if (!is_valid_) return false;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        LOG_ERROR("epoll删除fd=%d失败", fd);
        return false;
    }
    return true;
}

int Epoll::wait(struct epoll_event* events, int max_events, int timeout) {
    if (!is_valid_) return -1;
    return epoll_wait(epoll_fd_, events, max_events, timeout);
}