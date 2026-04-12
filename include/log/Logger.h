#ifndef LOGGER_H
#define LOGGER_H
#pragma once

#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <cstdarg>

class Logger {
public:
    static Logger& getInstance();
    void log(const std::string& level, const std::string& file, int line, const char* format, ...);
    ~Logger();

private:
    Logger();

    void backgroundThread();    // 后台线程函数，负责实际输出

    std::queue<std::string> msgQueue;   // 待输出的消息队列
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> stop { false };
};

#define LOG_INFO(fmt, ...) Logger::getInstance().log("INFO",  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::getInstance().log("ERROR", __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
