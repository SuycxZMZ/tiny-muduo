add_executable(AsyncLoggingTest asynclogtest.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/log/test)

target_link_libraries(AsyncLoggingTest /home/suycx/Code/tiny-muduo/lib/libtinymuduo.so pthread)