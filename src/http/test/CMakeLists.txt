add_executable(httpserver httpservertest.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/http/test)

target_link_libraries(httpserver /home/suycx/Code/tiny-muduo/lib/libtinymuduo.so pthread)