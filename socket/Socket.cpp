//
// Created by marvinle on 2019/2/1 4:21 PM.
//

#include "Socket.h"
#include "../Util/Util.h"
#include <cstring>
#include <cstdio>

ServerSocket::ServerSocket(int port, const char *ip) : mPort(port), mIp(ip), m_user_count()
{
    // 初始化结构体（把结构体清零）
    bzero(&mAddr, sizeof(mAddr));
    // 填充服务端信息
    mAddr.sin_family = AF_INET; // AF_INET: IPV4
    mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mAddr.sin_port = htons(port);

    // 创建监听套接字
    s_server = socket(AF_INET, SOCK_STREAM, 0);
    if (s_server == -1)
    {
        std::cout << "creat socket error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        exit(0);
    }

    // 设置socket属性, 让端口释放后立即就可以被再次使用
    setReusePort(s_server);
    setnonblocking(s_server);
}

// 绑定ip地址、端口等信息到socket上
void ServerSocket::bind()
{
    int ret = ::bind(s_server, (struct sockaddr *)&mAddr, sizeof(mAddr));
    if (ret == -1)
    {
        std::cout << "bind error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        exit(0);
    }
}

// 监听
void ServerSocket::listen()
{
    int ret = ::listen(s_server, 1024);
    if (ret == -1)
    {
        std::cout << "listen error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        exit(0);
    }
    // cout << "服务端正在监听连接...." << endl;
}

// 接收客户端上来的连接
int ServerSocket::accept(ClientSocket &clientSocket) const
{
    int clientfd = ::accept(s_server, (struct sockaddr *)&clientSocket.mAddr, &clientSocket.mLen);

    if (clientfd < 0)
    {
        if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            return clientfd;
        std::cout << "accept error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ << std::endl;
        std::cout << "clientfd:" << clientfd << std::endl;
        perror("accpet error");
    }

    clientSocket.fd = clientfd;
    return clientfd;
}

// 关闭连接
void ServerSocket::close()
{
    if (s_server >= 0)
    {
        ::close(s_server);
        s_server = -1;
    }
}

// 析构函数
ServerSocket::~ServerSocket()
{
    close();
}

// 关闭客户端连接
void ClientSocket::close()
{
    if (fd >= 0)
    {
        ::close(fd);
        fd = -1;
    }
}

// 析构函数
ClientSocket::~ClientSocket()
{
    close();
}
