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
    std::cout<<"线程["<<std::this_thread::get_id()<<"]开始处理fd:"<<client_fd<<std::endl;
    
    char buffer[1024]={0};
    ssize_t bytes_read=read(client_fd,buffer,1024);
    if(bytes_read>0){
        std::cout << "来自 FD " << client_fd << " 的消息: " << buffer;
        send(client_fd,buffer,bytes_read,0);

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLONESHOT; // 建议加上 ONESHOT
        ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
    }
    else{
            std::cout << "客户端 FD: " << client_fd << " 已断开。" << std::endl;
            close(client_fd);
    }

    

}

int main(){
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    std::cout<<server_fd<<std::endl;
    //这个是文件描述符
    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(8080);
    //一个结构体，存储信息，定义包括使用ipv4,ip地址和端口号
    bind(server_fd,(struct sockaddr*)&address,sizeof(address));
    listen(server_fd,128);
    std::cout << "Server 正在 8080 端口等待连接..." << std::endl;
    //创建epoll
    int epoll_fd=epoll_create1(0);
    struct epoll_event event;
    struct epoll_event events[1024];
    event.events=EPOLLIN;
    event.data.fd=server_fd;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&event);

    Threadpool pool(2);
    while(true){
        int nfds=epoll_wait(epoll_fd,events,1024,-1);
        std::cout<<nfds<<"个动静"<<std::endl;

        for(int i=0;i<nfds;i++){
            if(events[i].data.fd==server_fd){
                sockaddr_in client_addr;
                socklen_t addr_len=sizeof(client_addr);
                int client_fd=accept(server_fd,(struct sockaddr*)&client_addr,&addr_len);
                std::cout << "主线程：接受了新连接 FD: " << client_fd<< std::endl;

                event.events=EPOLLIN;
                event.data.fd=client_fd;
                epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&event);
            }
            else{
                int client_fd=events[i].data.fd;
                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,nullptr);
                pool.enqueue([client_fd,epoll_fd]{handle_client(client_fd,epoll_fd);});
            }
        }
    }
    
    close(epoll_fd);
    close(server_fd);

    return 0;
}