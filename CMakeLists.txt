cmake_minimum_required(VERSION 3.0)

project(tinymuduo)

add_definitions(-DMUDUODEBUG=0)
add_definitions(-DMUDUOINFO=1)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src/base)
include_directories(${PROJECT_SOURCE_DIR}/src/log)
include_directories(${PROJECT_SOURCE_DIR}/src/net)
include_directories(${PROJECT_SOURCE_DIR}/src/http)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

add_subdirectory(src)
add_subdirectory(src/log/test)
add_subdirectory(src/http/test)
