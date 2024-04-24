// utils.cc
#include "utils.h"
// #include <chrono> // For std::chrono
// #include <thread> // For std::this_thread

// Define file_logger
std::shared_ptr<spdlog::logger> file_logger; // = spdlog::rotating_logger_mt<spdlog::async_factory>("log_info", "/usr/local/tiny-muduo-log/log_info.log", 1024 * 1024 * 5, 10);

spdLoggerInit::spdLoggerInit()
{
    init();
}

spdLoggerInit::~spdLoggerInit()
{
}

// Definition of the init() function
void init() {
    spdlog::init_thread_pool(10000, 1);
    // Initialize logger
    file_logger = spdlog::rotating_logger_mt<spdlog::async_factory>("log_info", "/usr/local/tiny-muduo-log/log_info.log", 1024 * 1024 * 5, 10);
    // file_logger->set_level(spdlog::level::debug);
    file_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");
}

spdLoggerInit logger_init;

// int main() {

//     // Log some messages
//     LOG_INFO("hello world -----------");
//     LOG_INFO("hello world -----------");
//     LOG_INFO("hello world -----------");
//     LOG_INFO("hello world -----------");
//     LOG_INFO("hello world -----------");

//     // Other logging and program logic...

//     // Shutdown spdlog to ensure all logs are written before program exits
//     // spdlog::shutdown();

//     return 0;
// }

