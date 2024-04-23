// #ifndef UTILS_H
// #define UTILS_H 
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "noncopyable.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h" // support for basic file logging
#include "spdlog/sinks/rotating_file_sink.h" // support for rotating file logging

std::shared_ptr<spdlog::logger> file_logger = spdlog::rotating_logger_mt("log_info", "/usr/local/tiny-muduo-log/log_info.log", 1024 * 1024, 10);
//  file_logger_debug = spdlog::rotating_logger_mt("log_debug", "log_debug.log", 10 * 1024, 5);
// 设置日志格式. 参数含义: [日志标识符] [日期] [日志级别] [线程号] [文件名 函数名:行号] [数据]
// file_logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s %!:%#]  %v");

struct logInit : noncopyable
{
    logInit(){
        int a = 1;
        //[日志标识符] [日期] [日志级别] [线程号] [数据]
        file_logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        file_logger->set_level(spdlog::level::debug);
    }
};

logInit log_init;



// #endif