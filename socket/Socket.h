//#ifndef WEBSERVER_SOCKET_H
//#define WEBSERVER_SOCKET_H

#pragma once

// #include "../Util/Timer.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <memory>

class ClientSocket;

// 服务器端套接字
class ServerSocket
{
public:
    ServerSocket(int port = 8080, const char *ip = nullptr); // 在构造函数中获得fd

    ~ServerSocket();

    void bind();

    void listen();

    void close();

    int accept(ClientSocket &) const;

public:
    // struct sockaddr_in: 结构体用来处理网络通信的地址
    sockaddr_in mAddr; // 服务端地址
    int listenfd;      // 服务端套接字
    int epoll_fd;      // 内核事件表
    int mPort;         // 端口
    const char *mIp;   // ip
    int m_user_count;  // 用户数量
};

// 客户端套接字
class ClientSocket
{
public:
    ClientSocket() { fd = -1; };

    void close();

    ~ClientSocket();

    socklen_t mLen;
    sockaddr_in mAddr; // 客户端地址
    int fd;            // 客户端套接字
};
//#endif //WEBSERVER_SOCKET_H
