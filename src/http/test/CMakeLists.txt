add_executable(httpserver httpservertest.cc)
add_executable(async-httpserver async-httpserver.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/http/test)

target_link_libraries(httpserver ${PROJECT_SOURCE_DIR}/lib/libtinymuduo.so pthread)
target_link_libraries(async-httpserver ${PROJECT_SOURCE_DIR}/lib/libtinymuduo.so pthread)