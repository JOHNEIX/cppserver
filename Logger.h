#pragma once
#include<string>
#include<mutex>
#include <cstdarg>
class Logger{

public:
    static Logger& getInstance();//单例模式的设计
    void log(const std::string& level,
             const std::string& file,
             int line,
             const char* format,...);

private:
    Logger() =default;
    std::mutex mtx;    
};

#define LOG_INFO(fmt, ...) Logger::getInstance().log("INFO", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt,...) Logger::getInstance().log("ERROR", __FILE__, __LINE__, fmt, ##__VA_ARGS__)