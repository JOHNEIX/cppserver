#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
//将之前放在主文件中的类提取并封装。
class Threadpool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    
    Threadpool(size_t threads);
    ~Threadpool();

    void enqueue(std::function<void()> task);
};

#endif