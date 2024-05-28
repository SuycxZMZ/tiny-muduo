add_executable(asyncLoggingTest asynclogtest.cc)
add_executable(logtest logtest.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/log/test)

target_link_libraries(asyncLoggingTest ${PROJECT_SOURCE_DIR}/lib/libtinymuduo.so pthread)
target_link_libraries(logtest ${PROJECT_SOURCE_DIR}/lib/libtinymuduo.so pthread)