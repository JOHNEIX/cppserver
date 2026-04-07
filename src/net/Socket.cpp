#include "net/Socket.h"
#include <unistd.h>
#include <cstring>
#include "log/Logger.h"

Socket::Socket(){
    fd_=socket(AF_INET,SOCK_STREAM,0);
    if(fd_==-1){
        LOG_ERROR("创建socket失败");
        is_valid_=false;
        return;
    }
    is_valid_=true;
    LOG_INFO("创建服务器FD %d",fd_);
}

Socket::~Socket(){
    close(fd_);
    LOG_INFO("socket关闭,FD= %d ",fd_);
}

bool Socket::bind_listen(int port,int numbers){
    //一个结构体，存储信息，定义包括使用ipv4,ip地址和端口号
    if(!is_valid()){
        LOG_ERROR("socket无效,无法绑定");
        return false;
    }
    addr_.sin_family=AF_INET;
    addr_.sin_addr.s_addr=INADDR_ANY;
    addr_.sin_port=htons(port);

    if(bind(fd_,(struct sockaddr*)&addr_,sizeof(addr_))==-1){
        LOG_ERROR("绑定端口8080失败");
        close(fd_);
        return false;
    }   

    if(listen(fd_,numbers)==-1){
        LOG_ERROR("监听失败");
        close(fd_);
        return false;
    }

     LOG_INFO("服务器监听端口 %d 成功", port);
     return true;
}

int Socket::accept(){
    if (!is_valid_) return -1;
    int client_fd=::accept(fd_,nullptr,nullptr);
    return client_fd;
}

int Socket::get_fd() const {
    return fd_;
}