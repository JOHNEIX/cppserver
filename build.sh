#!/bin/bash
g++ src/net/reactorserver.cpp src/log/Logger.cpp src/thread/ThreadPool.cpp src/net/Epoll.cpp src/net/Socket.cpp -Iinclude -o server -pthread
./server