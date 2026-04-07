#include "log/Logger.h"
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <errno.h>

Logger& Logger::getInstance(){
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& level, const std::string& file, int line, const char* format, ...){
    std::lock_guard<std::mutex> lock(mtx);

    time_t now=time(0);
    struct tm* tstruct = localtime(&now);

    char timeBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tstruct);

    printf("[%s] [%s] [%s:%d] ", timeBuf, level.c_str(), file.c_str(), line);

    // 2. 处理变长参数 (类似 printf)
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    if(level=="ERROR"&&errno!=0){
        printf(" | 系统错误: %s", strerror(errno));
    }
    printf("\n");


}