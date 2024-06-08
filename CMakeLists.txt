cmake_minimum_required(VERSION 3.10)

project(tinymuduo)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

include_directories(${PROJECT_SOURCE_DIR}/src/base)
include_directories(${PROJECT_SOURCE_DIR}/src/log)
include_directories(${PROJECT_SOURCE_DIR}/src/net)
include_directories(${PROJECT_SOURCE_DIR}/src/http)

file(GLOB SRC_LIST
    # 列出所有需要编译的源文件
    src/base/*.cc
    src/log/*.cc
    src/net/*.cc
    src/http/*.cc
)

# add_library(tinymuduo STATIC ${SRC_LIST})
add_library(tinymuduo SHARED ${SRC_LIST})

# 设置安装前缀
set(CMAKE_INSTALL_PREFIX /usr/local)

# 递归查找所有头文件并安装到同一个目录中
file(GLOB_RECURSE HEADER_FILES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
)

# 添加头文件安装规则
install(FILES ${HEADER_FILES}
    DESTINATION include/tinymuduo
)

# 添加库文件安装规则
install(TARGETS tinymuduo
    LIBRARY DESTINATION lib
)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/http/test)
