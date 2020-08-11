//
// Created by marvinle on 2019/2/26 5:11 PM.
//

//#ifndef WEBSERVER_EVENT_H
//#define WEBSERVER_EVENT_H
#pragma once

#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "../http/HttpData.h"
#include "../socket/Socket.h"
#include "../Util/Timer.h"

enum EventType
{
    EIN = EPOLLIN,       // 写事件
    EOUT = EPOLLOUT,     // 读事件
    ECLOSE = EPOLLRDHUP, // 对端关闭连接或者写半部
    EPRI = EPOLLPRI,     // 紧急数据到达
    EERR = EPOLLERR,     // 错误事件
    EET = EPOLLET,       // 边缘触发
    EDEFULT = EIN | ECLOSE | EERR | EET
};

class Epoll
{
public:
    static int init(int max_events);

    static int addfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<HttpData>);

    static int modfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<HttpData>);

    static int delfd(int epoll_fd, int fd, __uint32_t events);

    static void handleConnection(const ServerSocket &serverSocket);

    static std::vector<std::shared_ptr<HttpData>> poll(const ServerSocket &serverSocket, int max_event, int timeout);

public:
    // httpmap中存储的是{文件描述符：对应的httpdata}
    static std::unordered_map<int, std::shared_ptr<HttpData>> httpDataMap;
    static const int MAX_EVENTS; // 最大事件数
    static const int MAX_FD;     //最大文件描述符
    static epoll_event *events;  // epoll_event结构体
    static TimerManager timerManager;
    const static __uint32_t DEFAULT_EVENTS;
};

//#endif //WEBSERVER_EVENT_H
