//#ifndef WEBSERVER_HTTPDATA_H
//#define WEBSERVER_HTTPDATA_H

#pragma once

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

    void closeTimer()
    {
        // 首先判断Timer是否还在， 有可能已经超时释放
        if (timer_.lock())
        {
            std::shared_ptr<TimerNode> tempTimer(timer_.lock());
            tempTimer->deleted();
            // 断开weak_ptr
            timer_.reset();
        }
    }

    void setTimer(std::shared_ptr<TimerNode> timer)
    {
        timer_ = timer;
    }

private:
    std::weak_ptr<TimerNode> timer_; // TimeNode类型的智能指针
};

//#endif //WEBSERVER_HTTPDATA_H
