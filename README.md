### 简要说明
与原版muduo库相比：
    移除了boost库，使用c++11标准库代替。
    只用了epoll实现主要功能。


### 1. 编译&&安装 

    硬性要求：linux环境，C++编译器支持C++11标准。
    测试环境：
        ubuntu20.04,
        g++ 9.3.0,
        GNU Make 4.2.1,
        cmake version 3.16.3
    
    编译&&安装：
        git clone https://github.com/SuycxZMZ/tiny-muduo.git
        cd tiny-muduo
        需要root权限，在工程目录下执行：
        sh ./autobuild.sh
    测试回显服务器：
        cd example
        make clean
        make
        ./echoserver
        ## 正常安装的话已经开始打印信息了，并等待客户端连接
    测试回显客户端：
        telnet 127.0.0.1 8000
        ## 输入任意字符，回车后，会打印出客户端输入的字符

### 2. 主要组件说明

[1. 主要组件说明](allmd/basicClass.md)

[2. 连接流程说明](allmd/basicConnectModel.md)

[3. 读写流程说明](allmd/basicReadWriteModel.md)

[4. 关闭流程说明](allmd/basicCloseModel.md)

[5. one loop per thread 模型](allmd/basicOneLoopPerThreadModel.md)

[6. 日志模块说明](allmd/basiclog.md)

### 3. 压力测试 ApacheBench

    ---- 控制台日志 ----
        ab -n 100000 -c 100 -k http://localhost:8000/
        环境：ubuntu20.04, 2核2G, 起2个线程和4个线程基本一样
        打开LOG_INFO, qps:18000左右
        关闭所有LOG, qps:70000左右
        控制台日志每次都切内核标准输出，请求量大的时候效率极低

    ---- 日志模块，单独起线程和缓冲区buffer ----

参考 && 致谢 ：

https://github.com/chenshuo/muduo

https://blog.csdn.net/T_Solotov/article/details/124044175

https://zhuanlan.zhihu.com/p/636581210

https://github.com/Shangyizhou/A-Tiny-Network-Library

https://www.cnblogs.com/tuilk/p/16793625.html