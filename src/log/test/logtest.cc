#include "asynclogging.h"
#include "logging.h"
#include "timestamp.h"

#include <stdio.h>
#include <unistd.h>

void test_Logging()
{
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";
    // 注意不能轻易使用 LOG_FATAL, LOG_SYSFATAL, 会导致程序abort

    const int n = 10;
    for (int i = 0; i < n; ++i) {
        LOG_INFO << "Hello, " << i << " abc...xyz";
    }
}


int main(int argc, char* argv[])
{
    printf("pid = %d\n", getpid());
    // AsyncLogging log(::basename(argv[0]), kRollSize);
    test_Logging();
    sleep(1);
    return 0;
}