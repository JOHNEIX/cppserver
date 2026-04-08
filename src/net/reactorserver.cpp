#include "net/EventLoop.h"
#include "net/Channel.h"
#include "log/Logger.h"
#include "thread/ThreadPool.h"
#include "net/Socket.h"
#include "net/Acceptor.h"
#include "net/Epoll.h"
#include <unistd.h>

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

    //std::cout << "Server 正在 8080 端口等待连接..." << std::endl;

    // 2. 创建反应堆
    EventLoop loop;
    Threadpool pool(2);

    Acceptor acceptor(&loop, 8080, &pool);

    loop.loop();


    return 0;
}