//
// Created by marvinle on 2019/2/26 3:19 PM.
//
//#ifndef WEBSERVER_TIMER_H
//#define WEBSERVER_TIMER_H

#pragma once

#include "../http/HttpData.h"
#include "MutexLock.h"

#include <queue>
#include <deque>
#include <memory>

class HttpData;

// 时间节点
class TimerNode
{
public:
    TimerNode(std::shared_ptr<HttpData> httpData, size_t timeout);
    ~TimerNode();

public:
    bool isDeleted() const { return deleted_; }

    // 得到当前还剩余的时间
    size_t getExpireTime() { return expiredTime_; }

    // 频繁调用系统调用不好
    //current_time();
    // 判断事件是否已经过期了
    bool isExpire()
    {
        return expiredTime_ < current_msec;
    }

    void deleted();

    static void current_time();

    std::shared_ptr<HttpData> getHttpData() { return httpData_; }

    static size_t current_msec; // 当前时间

private:
    bool deleted_;
    size_t expiredTime_; // 毫秒
    std::shared_ptr<HttpData> httpData_;
};

// 时间对比
struct TimerCmp
{
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
    {
        return a->getExpireTime() > b->getExpireTime();
    }
};

// 时间管理
class TimerManager
{
public:
    // typedef std::shared_ptr<TimerNode> Shared_TimerNode;
    using Shared_TimerNode = std::shared_ptr<TimerNode>;

public:
    void addTimer(std::shared_ptr<HttpData> httpData, size_t timeout);

    void handle_expired_event();

    const static size_t DEFAULT_TIME_OUT;

private:
    // 优先队列，按照还剩的时间来排序，小顶堆
    std::priority_queue<Shared_TimerNode, std::deque<Shared_TimerNode>, TimerCmp> TimerQueue; // 超时队列
    MutexLock lock_;                                                                          // 互斥锁
};

//#endif //WEBSERVER_TIMER_H
