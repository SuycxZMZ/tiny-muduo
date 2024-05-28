add_executable(asyncLoggingTest asynclogtest.cc)
add_executable(logtest logtest.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/log/test)

target_link_libraries(asyncLoggingTest /home/suycx/Code/tiny-muduo/lib/libtinymuduo.so pthread)
target_link_libraries(logtest /home/suycx/Code/tiny-muduo/lib/libtinymuduo.so pthread)