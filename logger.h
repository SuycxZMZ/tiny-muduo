#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include "noncopyable.h"

// log宏 LOG_INFO(formatstr, arg1, arg2, ...)
#define LOG_INFO(logMsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(INFO);                         \
        char buf[1024];                                   \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(std::string(buf));                     \
    } while (0)

#define LOG_ERROR(logMsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(ERROR);                        \
        char buf[1024];                                   \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(std::string(buf));                     \
    } while (0)
#ifdef MUDUODEBUG
#define LOG_DEBUG(logMsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(DEBUG);                        \
        char buf[1024];                                   \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(std::string(buf));                     \
    } while (0)
#else
#define LOG_DEBUG(logMsgFormat, ...)
#endif

#define LOG_FATAL(logMsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::instance();              \
        logger.setLogLevel(FATAL);                        \
        char buf[1024];                                   \
        snprintf(buf, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(std::string(buf));                     \
    } while (0)

// 日志级别
enum loglevel
{
    INFO,
    ERROR,
    DEBUG,
    FATAL
};

// 日志类，单例
class Logger : noncopyable
{
public:
    // 获取日志单例
    static Logger &instance();
    // 设置日志级别
    void setLogLevel(int level);
    // 写日志
    void log(std::string log_msg);

private:
    // 级别
    int m_level;
    Logger() {}
};

#endif