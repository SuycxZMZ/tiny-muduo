// utils.h
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"

// Declare file_logger as an extern variable
extern std::shared_ptr<spdlog::logger> file_logger;

class spdLoggerInit
{
public:
    spdLoggerInit();
    ~spdLoggerInit();
};
void init(); // Declaration of the initialization function


#ifdef CLOSEALLLOG
    #define LOG_INFO(logMsgFormat, ...)
    #define LOG_DEBUG(logMsgFormat, ...)
    #define LOG_ERROR(logMsgFormat, ...)
    #define LOG_FATAL(logMsgFormat, ...)
#else
    #define LOG_INFO(logMsgFormat, ...) \
        file_logger->info(logMsgFormat, ##__VA_ARGS__); file_logger->flush()
    #define LOG_DEBUG(logMsgFormat, ...) \
        file_logger->debug(logMsgFormat, ##__VA_ARGS__)
    #define LOG_ERROR(logMsgFormat, ...) \
            file_logger->error(logMsgFormat, ##__VA_ARGS__)
    #define LOG_FATAL(logMsgFormat, ...) \
            file_logger->critical(logMsgFormat, ##__VA_ARGS__)
#endif  

