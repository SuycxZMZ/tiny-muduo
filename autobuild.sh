#!/bin/bash

set -e # exit on error

# 检查build 和 bin 目录是否存在，不存在则创建
if [ ! -d `build` ]; then
    mkdir build
    echo "build 目录创建成功"

fi

if [ ! -d `lib` ]; then
    mkdir lib
    echo "lib 目录创建成功"  
fi

# 检查在example文件夹下是否存在bin目录,如果不存在,则创建并使 bin 目录的权限为 777
if [ ! -d `pwd`/example/bin ]; then
    mkdir `pwd`/example/bin
    chmod 777 `pwd`/example/bin
    echo "example/bin 目录创建成功"

fi

rm -rf `pwd`/build/* # clean build directory

cd `pwd`/build &&
    cmake .. &&
    make -j4
cd ..

# 把头文件拷贝到 /usr/include/tinymuduo  so库拷贝到 /usr/lib
# 注意：这里需要root权限
# 检查/usr/include/tinymuduo 目录是否存在，不存在则创建
if [ ! -d "/usr/include/tinymuduo" ]; then
    mkdir -p /usr/include/tinymuduo
    echo "/usr/include/tinymuduo 目录创建成功"
fi

cp -r `pwd`/src/base/*.h /usr/include/tinymuduo
cp -r `pwd`/src/log/*.h /usr/include/tinymuduo
cp -r `pwd`/src/net/*.h /usr/include/tinymuduo
cp -r `pwd`/src/http/*.h /usr/include/tinymuduo
cp `pwd`/lib/*.so /usr/lib

ldconfig

