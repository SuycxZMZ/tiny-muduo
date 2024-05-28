cmake_minimum_required(VERSION 3.0)

project(tinymuduo)

add_definitions(-DMUDUODEBUG=0)
add_definitions(-DMUDUOINFO=1)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

add_subdirectory(src)
