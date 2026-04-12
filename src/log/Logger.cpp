#include "log/Logger.h"
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <errno.h>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// 构造：启动后台输出线程
Logger::Logger() {
    worker = std::thread(&Logger::backgroundThread, this);
}

// 析构：通知后台线程退出，等待它把队列里剩余消息全部输出完
Logger::~Logger() {
    stop = true;
    cv.notify_one();
    if (worker.joinable())
        worker.join();
}

// 调用方线程：格式化消息 → 压队列 → 立刻返回
void Logger::log(const std::string& level, const std::string& file, int line, const char* format, ...) {
    // 捕获当前线程的 errno，因为后台线程输出时 errno 已经变了
    int savedErrno = errno;

    // 获取时间戳
    time_t now = time(0);
    struct tm tstruct;
    localtime_r(&now, &tstruct);
    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tstruct);

    // 格式化用户消息
    char msgBuf[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(msgBuf, sizeof(msgBuf), format, args);
    va_end(args);

    // 拼成完整一行
    char fullMsg[1200];
    int len = snprintf(fullMsg, sizeof(fullMsg), "[%s] [%s] [%s:%d] %s",
                       timeBuf, level.c_str(), file.c_str(), line, msgBuf);

    if (level == "ERROR" && savedErrno != 0) {
        snprintf(fullMsg + len, sizeof(fullMsg) - len, " | 系统错误: %s", strerror(savedErrno));
    }

    // 压入队列，通知后台线程
    {
        std::lock_guard<std::mutex> lock(mtx);
        msgQueue.push(std::string(fullMsg));
    }
    cv.notify_one();
}

// 后台线程：等待消息 → 输出 → 循环
void Logger::backgroundThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        // 等待：队列有消息，或者收到退出信号
        cv.wait(lock, [this] { return !msgQueue.empty() || stop.load(); });

        // 把当前队列里所有消息都取出来，避免长时间持锁
        std::queue<std::string> local;
        local.swap(msgQueue);
        lock.unlock();

        while (!local.empty()) {
            printf("%s\n", local.front().c_str());
            local.pop();
        }

        // stop 为 true 且队列已清空，退出
        if (stop && msgQueue.empty())
            break;
    }
}
