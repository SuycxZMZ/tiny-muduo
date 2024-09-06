cmake_minimum_required(VERSION 3.10)

project(tinymuduo)

# 开启生成 cpmpile_commands.json 
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/base
    ${PROJECT_SOURCE_DIR}/src/log
    ${PROJECT_SOURCE_DIR}/src/net
    ${PROJECT_SOURCE_DIR}/src/http
)

# 列出所有需要编译的源文件
file(GLOB SRC_LIST
    src/base/*.cc
    src/log/*.cc
    src/net/*.cc
    src/http/*.cc
)

add_library(tinymuduo SHARED ${SRC_LIST})

# 测试代码
option(BUILD_TEST "ON for compile test" ON)
if (BUILD_TEST)
    add_subdirectory(test)
endif()

# -------------------------------------- install --------------------------------------
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
