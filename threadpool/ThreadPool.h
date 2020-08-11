//
// Created by marvinle on 2019/2/25 11:26 PM.
//

//#ifndef WEBSERVER_THREADPOLL_H
//#define WEBSERVER_THREADPOLL_H

#pragma once

#include <vector>
#include <list>
#include <functional>
#include <thread>
#include <memory>

#include "../Util/noncopyable.h"
#include "../Util/MutexLock.h"
#include "../Util/Condition.h"

const int MAX_THREAD_SIZE = 1024;
const int MAX_QUEUE_SIZE = 10000; // 任务队列个数

typedef enum
{
    immediate_mode = 1,
    graceful_mode = 2
} ShutdownMode;

// 封装线程池中的对象需要执行的任务对象
struct ThreadTask
{
    // 回调函数
    std::function<void(std::shared_ptr<void>)> process;
    // 回调函数的参数
    std::shared_ptr<void> arg;
};

// 线程池
class ThreadPool : public noncopyable
{
public:
    ThreadPool(int thread_s, int max_queue_s);
    ~ThreadPool();

    void start();                 // 开启线程池
    void shutdown(bool graceful); // 关闭线程池
    bool append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> fun);

private:
    static void *worker(void *args); // 线程运行函数，执行run函数
    void run();                      // 线程工作函数
    bool isFull();                   // 任务队列是否已满

    // 线程同步互斥, mutex_ 在 condition_前面
    MutexLock mutex_;     // 锁（同步
    Condition condition_; // 信号量（条件阻塞

    // 线程池属性
    int thread_size;                     // 线程数
    int max_queue_size;                  // 最大任务队列
    int shutdown_;                       // 判断线程池是否关闭，且关闭模式是什么
    int idle;                            // 线程池中空闲线程数
    std::vector<std::thread> _threads;   // 线程池
    std::list<ThreadTask> request_queue; // 任务队列

    // int running_;
    // int started;                         //判断线程池是否运行
    // using Task = std::function<void()>;
};

// 线程类
// class Thread
// {

// public:
// };

//#endif //WEBSERVER_THREADPOLL_H
