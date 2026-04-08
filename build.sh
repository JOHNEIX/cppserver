#!/bin/bash
g++ src/net/reactorserver.cpp src/log/Logger.cpp src/thread/ThreadPool.cpp src/net/EventLoop.cpp src/net/Socket.cpp src/net/Acceptor.cpp src/net/Channel.cpp src/net/Epoll.cpp -Iinclude -o server -pthread
./server