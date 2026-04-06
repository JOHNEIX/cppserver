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
#include "Logger.h"
class Threadpool{
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>>tasks;
        std::mutex queue_mutex; //队列锁
        std::condition_variable condition;
        bool stop;// 是否停止工作的标志

    public:
        Threadpool(size_t threads):stop(false){
            for(size_t i=0;i<threads;i++){
                workers.emplace_back([this]{
                    while(true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,[this]{
                                return this->stop||!this->tasks.empty();
                            });
                            if(this->stop&&this->tasks.empty()) return;
                            task=std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        void enqueue(std::function<void()> task){
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                tasks.emplace(std::move(task));
            }
            condition.notify_one();
        }

        ~Threadpool(){
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                stop=true;
            }
            condition.notify_all();
            for(std::thread& worker:workers) worker.join();

        }

};

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
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd==-1){
        LOG_ERROR("创建socket失败");
        return 1;
    }
    LOG_INFO("创建服务器FD %d",server_fd);
    //这个是文件描述符
    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(8080);
    //一个结构体，存储信息，定义包括使用ipv4,ip地址和端口号
    
    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))==-1){
        LOG_ERROR("绑定端口8080失败");
        close(server_fd);
        return 1;
    }

    if(listen(server_fd,128)==-1){
        LOG_ERROR("监听失败");
        close(server_fd);
        return 1;
    }
    std::cout << "Server 正在 8080 端口等待连接..." << std::endl;

    //创建epoll
    int epoll_fd=epoll_create1(0);
    if(epoll_fd==-1){
        LOG_ERROR("创建epoll实例失败");
        close(server_fd);
        return 1;
    }
    struct epoll_event event;
    struct epoll_event events[1024];
    event.events=EPOLLIN;
    event.data.fd=server_fd;
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&event)==-1){
        LOG_ERROR("epoll_ctl加入服务端fd失败");
        close(epoll_fd);
        close(server_fd);
        return 1;
    }
    
    Threadpool pool(2);
    while(true){
        int nfds=epoll_wait(epoll_fd,events,1024,-1);
        //std::cout<<nfds<<"个动静"<<std::endl;

        for(int i=0;i<nfds;i++){
            int fd=events[i].data.fd;
            if(fd==server_fd){
                sockaddr_in client_addr;
                socklen_t addr_len=sizeof(client_addr);
                int client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&addr_len);
                if(client_fd==-1){
                    LOG_ERROR("客户端连接失败,错误码: %d",errno);
                    continue;
                }
                LOG_INFO("新客户端接入,FD=%d", client_fd);

                struct epoll_event ev;
                ev.events=EPOLLIN|EPOLLONESHOT;
                ev.data.fd=client_fd;
                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&ev)==-1){
                    LOG_ERROR("epoll_ctl加入客户端FD %d 失败",client_fd);
                    close(client_fd);
                }
            }
            else{
                pool.enqueue([fd,epoll_fd]{handle_client(fd,epoll_fd);});
            }
        }
    }
    
    close(epoll_fd);
    close(server_fd);

    return 0;
}