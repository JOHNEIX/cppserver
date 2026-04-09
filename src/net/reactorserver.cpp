#include "net/EventLoop.h"
#include "net/Acceptor.h"
#include "thread/ThreadPool.h"
#include "log/Logger.h"


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