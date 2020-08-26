#include <sys/time.h>
#include <unistd.h>
#include "../Epoll/Epoll.h"
#include "Timer.h"

size_t TimerNode::current_msec = 0; // 当前时间

const size_t TimerManager::DEFAULT_TIME_OUT = 20 * 1000; // 20s

// 初始化列表；构造函数
TimerNode::TimerNode(std::shared_ptr<HttpData> httpData, size_t timeout) : deleted_(false), httpData_(httpData)
{
    current_time();                        // 获得当前时间
    expiredTime_ = current_msec + timeout; // 当前时间加上timeout
}

// 析构
TimerNode::~TimerNode()
{
    // FIXME 析构关闭资源的时候，要将httpDataMap中的引用,否则资源无法关闭，后期可改进为httpDataMap存储 weak_ptr<HttpData>
    // std::cout << "TimerNode析构" << std::endl;
    // 析构时如果是被deleted 则httpData为NULL, 不用处理，而如果是超时，则需要删除Epoll中的httpDataMap中的fd
    if (httpData_)
    {
        auto it = Epoll::users.find(httpData_->clientSocket_->fd);
        if (it != Epoll::users.end())
        {
            Epoll::users.erase(it);
        }
    }
}

// 当前时间
void inline TimerNode::current_time()
{
    struct timeval cur;
    gettimeofday(&cur, NULL);
    current_msec = (cur.tv_sec * 1000) + (cur.tv_usec / 1000);
}

// 删除
void TimerNode::deleted()
{
    // 删除采用标记删除， 并及时析构HttpData，以关闭描述符
    // 关闭定时器时应该把 httpDataMap 里的HttpData 一起erase
    httpData_.reset();
    deleted_ = true;
}

// 增加时间节点
void TimerManager::addTimer(std::shared_ptr<HttpData> httpData, size_t timeout)
{
    // Shared_TimerNode = std::shared_ptr<TimerNode>
    Shared_TimerNode timerNode(new TimerNode(httpData, timeout));
    {
        MutexLockGuard guard(lock_);
        TimerQueue.push(timerNode);
        // 将TimerNode和HttpData关联起来
        httpData->setTimer(timerNode);
    }
}

// 处理超时的事件
void TimerManager::handle_expired_event()
{
    MutexLockGuard guard(lock_);
    // 更新当前时间
    //std::cout << "开始处理超时事件" << std::endl;
    TimerNode::current_time();
    while (!TimerQueue.empty())
    {
        Shared_TimerNode timerNode = TimerQueue.top();
        if (timerNode->isDeleted())
        {
            // 删除节点
            TimerQueue.pop();
        }
        else if (timerNode->isExpire())
        {
            // 过期 删除
            TimerQueue.pop();
        }
        else
        {
            break;
        }
    }
}
