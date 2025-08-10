#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include "noncopyable.h"


//##__VA_ARGS__表示可变参数列表
//LOG_INFO("%s %d", arg1, arg2)
#define LOG_INFO(logmsgFormat, ...)\
    do\
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024]; \
        snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)


#define LOG_ERROR(logmsgFormat, ...)\
    do\
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024]; \
        snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_FATAL(logmsgFormat, ...)\
    do\
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024]; \
        snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    } while(0)
#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)\
    do\
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024]; \
        snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)
#else
    #define LOG_DEBUG(logmsgFormat, ...)
#endif
//定义日志的级别 INFO ERROR FATAL DEBUG
enum LogLevel{
    INFO = 0,//普通信息
    ERROR = 1,//错误信息
    FATAL = 2,//coredump信息
    DEBUG = 3//调试信息
};

//输出一个日志类
class Logger: Noncopyable{
public:
    static Logger& instance();
    void setLogLevel(int level);
    void log(const std::string& msg);
private:
    Logger();
    int logLevel_;


    Logger(const Logger&)=delete;
    Logger& operator=(const Logger&)=delete;

};
