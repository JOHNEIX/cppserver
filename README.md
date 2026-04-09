# ReactorServer

基于 epoll + 线程池的 C++ Reactor 模式 TCP 服务器，目前实现了一个简单的回显服务。

## 架构

主线程运行 EventLoop，Acceptor 接受新连接并注册到 epoll，客户端可读事件触发后将 I/O 任务投递到 Threadpool 处理。

## 运行

```bash
./build.sh  # 编译并启动，监听 8080 端口
```

## 测试

```bash
nc 127.0.0.1 8080
```
