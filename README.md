### 简要说明
    与原版muduo库相比：
    移除了boost库，使用c++11标准库代替。
    只用了epoll实现主要功能。
    现阶段只实现了单buffer缓冲区

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

[1. 主要组件说明](basicClass.md)

[2. 连接流程说明](basicConnectModel.md)

[3. 读写流程说明](basicReadWriteModel.md)

参考 && 致谢 ：

https://github.com/chenshuo/muduo

https://blog.csdn.net/T_Solotov/article/details/124044175

https://zhuanlan.zhihu.com/p/636581210