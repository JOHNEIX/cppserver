#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>

class Socket{

public:
    Socket();
    ~Socket();
    //将绑定和监听整合
    bool bind_listen(int port,int numbers=128);
    //接收新链接
    int accept();
    //
    int get_fd() const;

    bool is_valid(){ return is_valid_; }
private:
    int fd_;
    sockaddr_in addr_;
    bool is_valid_;
};

#endif