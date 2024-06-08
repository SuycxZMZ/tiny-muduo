#include "tinymuduo.h"

#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

// 计算时间
class TimeLogger {
   public:
    TimeLogger() {
        time1 = std::chrono::high_resolution_clock::now();  // 构造时记录开始时间
    }

    ~TimeLogger() {
        time2 = std::chrono::high_resolution_clock::now();  // 析构时记录结束时间
        std::chrono::duration<double> duration = time2 - time1;
        // 计算当前文件夹下所有 .log 文件的总大小
        size_t total_size = 0;
        for (const auto& entry : fs::directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                total_size += entry.file_size();
            }
        }

        double rate = total_size / duration.count();
        printf("Total log size: %zu bytes\n", total_size);
        printf("Logging 2000000 entries took %f seconds\n", duration.count());
        printf("Rate: %.6f MB/second\n", rate / (1024 * 1024));
    }

   public:
    std::chrono::high_resolution_clock::time_point time1;
    std::chrono::high_resolution_clock::time_point time2;
};

void test_SyncLogging() {
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";
    // 注意不能轻易使用 LOG_FATAL, LOG_SYSFATAL, 会导致程序abort
    for (int i = 0; i < 10; ++i) {
        LOG_INFO << "SyncLogging test " << i;
    }
}

void test_AsyncLogging() {
    const int cnt = 1000 * 1000;
    for (int i = 0; i < cnt; ++i) {
        LOG_INFO << "AsyncLogging test " << i;
    }
}

TimeLogger timeLogger;

int main(int argc, char* argv[]) {
    printf("pid = %d\n", getpid());
    test_SyncLogging();

    tinymuduo::initAsyncLogging(::basename(argv[0]), 1024 * 1024 * 500);
    tinymuduo::AsyncLogStart();

    timeLogger.time1 = std::chrono::high_resolution_clock::now();

    std::thread t1(test_AsyncLogging);
    std::thread t2(test_AsyncLogging);
    t1.join();
    t2.join();

    return 0;
}