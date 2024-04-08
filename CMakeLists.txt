cmake_minimum_required(VERSION 3.0)
project(tiny-muduo)

# 设置要求的 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGES} -g)

# 配置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 头文件搜索路径 -I
# include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
# -I/usr/local/include/google/protobuf  -L/usr/local/lib
include_directories(/usr/local/include/google/protobuf)
include_directories(/usr/local/include/zookeeper)

# 设置项目库文件搜索路径 -L  protobuf zookeeper的连接库/usr/local/lib
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(/usr/local/lib)

# 加载子目录
add_subdirectory(src)
add_subdirectory(example)