## 安装
```shell
# ubuntu
sudo apt-get install apache2-utils
# CentOS
yum -y install httpd-tools

ab -V # 确认安装成功
```
## 参数查看
```shell
ab --help
```

简单测试
```shell
ab -n 10000 -c 1000 -k http://localhost:8000/
```
