//
// Created by marvinle on 2019/2/26 2:39 PM.
//
//#ifndef WEBSERVER_HTTPDATA_H
//#define WEBSERVER_HTTPDATA_H

#pragma once

#include "HttpParse.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "../socket/Socket.h"
#include "../Util/Timer.h"

#include <memory>

class TimerNode;

class HttpData : public std::enable_shared_from_this<HttpData>
{
public:
    // 构造函数
    HttpData() : epoll_fd(-1) {}

public:
    std::shared_ptr<HttpRequest> request_;       // 指向HttpRequest对象的指针
    std::shared_ptr<HttpResponse> response_;     // 指向HttpRreponse对象的指针
    std::shared_ptr<ClientSocket> clientSocket_; // 指向客户端Socket对象的指针

    int epoll_fd;

    void closeTimer();

    void setTimer(std::shared_ptr<TimerNode>);

private:
    std::weak_ptr<TimerNode> timer_; // TimeNode类型的智能指针
};

//#endif //WEBSERVER_HTTPDATA_H
