#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <sys/epoll.h>
#include "log/Logger.h"
#include "thread/ThreadPool.h"
#include "net/Socket.h"
#include "net/Epoll.h"

void handle_client(int client_fd,int epoll_fd){
    //std::cout<<"线程["<<std::this_thread::get_id()<<"]开始处理fd:"<<client_fd<<std::endl;
    LOG_INFO("线程[%zu] 开始处理FD %d", std::this_thread::get_id(), client_fd);//个人想看。
    char buffer[1024]={0};
    ssize_t bytes_read=read(client_fd,buffer,1024);
    if(bytes_read>0){
        LOG_INFO("收到 FD %d 的数据, 长度: %ld", client_fd, bytes_read);
        ssize_t bytes_send=send(client_fd,buffer,bytes_read,0);
        if(bytes_send==-1){
            LOG_ERROR("发送数据给FD %d 失败",client_fd);
            close(client_fd);//一般是连接异常，所以可以直接关闭
            return;
        }
        else if(bytes_send<bytes_read){
            LOG_ERROR("FD %d 数据未完全发送",client_fd);
        }
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLONESHOT; //ONESHOT,触发时屏蔽
        ev.data.fd = client_fd;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev)==-1){
            LOG_ERROR("epoll_ctl MOD 客户端fd失败");
        }    //重新装载
    }
    else if(bytes_read==0){
            LOG_INFO("客户端FD %d 已断开连接",client_fd);
            close(client_fd);
    }
    else{
        //等于-1的情况发生错误
        close(client_fd);
    }
    
}

int main(){
    LOG_INFO("服务器启动");
    Socket sock;
    if(!sock.is_valid()) return 1; 

    if(!sock.bind_listen(8080)) return 1;


    //std::cout << "Server 正在 8080 端口等待连接..." << std::endl;

    //创建epoll
    Epoll epoll;
    if(!epoll.is_valid()){
        return 1;
    }
    //监听fd加入epoll
    if(!epoll.add_fd(sock.get_fd(),EPOLLIN)){
         LOG_ERROR("epoll_ctl加入服务端fd失败");
         return 1;
    }
    
    struct epoll_event events[1024];

    Threadpool pool(2);

    while(true){
        int nfds=epoll.wait(events,1024);
        //std::cout<<nfds<<"个动静"<<std::endl;
        if(nfds<0){
            LOG_ERROR("epoll wait 失败");
            break;
        }
        for(int i=0;i<nfds;i++){
            int fd=events[i].data.fd;
            if(fd==sock.get_fd()){
            int fd=events[i].data.fd;
            if(fd==sock.get_fd()){
                int client_fd=sock.accept();
                epoll.add_fd(client_fd,EPOLLIN | EPOLLONESHOT);
            }
            }
            else{
                int epoll_fd=epoll.get_fd();
                pool.enqueue([fd,epoll_fd]{handle_client(fd,epoll_fd);});
            }
        }
    }

    return 0;
}