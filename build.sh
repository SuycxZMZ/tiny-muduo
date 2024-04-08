#!/bin/bash
set -x

# 检查是否存在 build 和 bin 文件夹，如果不存在则创建
mkdir -p build bin

# 清空 build 和 bin 文件夹中的内容
echo "清空 build 和 bin 文件夹中的内容"
rm -rf build/* bin/*

# 进入 build 文件夹并执行 cmake 和 make
cd build &&
cmake .. &&
make