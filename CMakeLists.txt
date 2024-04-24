cmake_minimum_required(VERSION 3.0)

project(tinymuduo)

add_definitions(-DMUDUODEBUG=0)
add_definitions(-DCLOSEALLLOG=0)
add_definitions(-DLOGLEVEL=1)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# set(INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/)
# set(INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/develop)
# include_directories(${PROJECT_SOURCE_DIR}/include)
# include_directories(${PROJECT_SOURCE_DIR}/include/spdlog)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

aux_source_directory(. SRC_LIST)

add_library(tinymuduo SHARED ${SRC_LIST})

# add_subdirectory(develop) 