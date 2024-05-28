## 安装
进入webbench-1.5源码文件夹
```shell
apt-get install exuberant-ctags
sudo make 
sudo make install
```


## 服务器压力测试
===============
Webbench是有名的网站压力测试工具，它是由[Lionbridge](http://www.lionbridge.com)公司开发。

> * 测试处在相同硬件上，不同服务的性能以及不同硬件上同一个服务的运行状况。
> * 展示服务器的两项内容：每秒钟响应请求数和每秒钟传输数据量。




测试规则
------------
* 测试示例

    ```C++
	webbench -c 500  -t  30   http://127.0.0.1:8080/
    ```
* 参数

> * `-c` 表示客户端数
> * `-t` 表示时间


测试结果
---------
Webbench对服务器进行压力测试，经压力测试可以实现上万的并发连接.
> * 并发连接总数：
> * 访问服务器时间：5s
> * 每秒钟响应请求数： pages/min
> * 每秒钟传输数据量： bytes/sec
> * 所有访问均成功

