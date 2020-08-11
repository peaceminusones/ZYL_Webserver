//
// Created by marvinle on 2019/2/26 9:42 AM.
//

#include "ThreadPool.h"
#include <iostream>
#include <assert.h>
#include <sys/prctl.h>

ThreadPool::ThreadPool(int thread_s, int max_queue_s) : thread_size(thread_s), max_queue_size(max_queue_s),
                                                        condition_(mutex_)
{
    // MAX_THREAD_SIZE: 最大线程数
    if (thread_s <= 0 || thread_s > MAX_THREAD_SIZE)
    {
        thread_size = 4;
    }

    // MAX_QUEUE_SIZE: 最大任务数
    if (max_queue_s <= 0 || max_queue_s > MAX_QUEUE_SIZE)
    {
        max_queue_size = MAX_QUEUE_SIZE;
    }

    // 分配空间
    _threads.resize(thread_size);
}

ThreadPool::~ThreadPool()
{
    if (!shutdown_)
        shutdown(1);
}

// 开启线程池
void ThreadPool::start()
{
    assert(_threads.empty());
    shutdown_ = 0;

    for (int i = 0; i < thread_size; i++)
    {
        _threads.push_back(std::thread(&ThreadPool::worker, this));
    }
}

// 关闭线程池
void ThreadPool::shutdown(bool graceful)
{
    {
        MutexLockGuard guard(this->mutex_);
        if (shutdown_)
        {
            std::cout << "has shutdown" << std::endl;
        }

        shutdown_ = graceful ? graceful_mode : immediate_mode;
        condition_.notifyAll();
    }

    for (int i = 0; i < _threads.size(); i++)
    {
        if (!_threads[i].joinable())
        {
            std::cout << "pthread_join error" << std::endl;
        }

        _threads[i].join();
    }
}

// 添加任务函数
// 添加一个任务到线程池，arg: 产生一个新的任务 fun: 任务函数
bool ThreadPool::append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> fun)
{
    if (shutdown_)
    {
        std::cout << "ThreadPool has shutdown" << std::endl;
        return false;
    }

    if (_threads.empty())
    {
        fun(arg);
        return false;
    }
    else
    {
        // 线程池被多个线程共享，操作前需要加锁
        MutexLockGuard guard(this->mutex_);
        // 判断任务队列是否满了，满了则添加失败
        if (isFull())
        {
            std::cout << max_queue_size;
            std::cout << "ThreadPool too many requests" << std::endl;
            return false;
        }

        // 产生一个新任务对象
        ThreadTask threadTask;
        // 任务参数为arg
        threadTask.arg = arg;
        // 任务函数为fun
        threadTask.process = fun;

        // 把任务添加到任务队列中
        request_queue.push_back(threadTask);

        // 唤醒线程池中等待任务的线程
        condition_.notify();

        return true;
    }
}

// 线程处理函数
// 工作线程运行的函数，它不断从工作队列中取出任务并执行
void *ThreadPool::worker(void *args)
{
    // 指向线程池的指针
    ThreadPool *pool = static_cast<ThreadPool *>(args);
    // 退出线程
    if (pool == nullptr)
        return NULL;
    prctl(PR_SET_NAME, "EventLoopThread"); // 设置线程名字

    // 执行线程主方法
    pool->run();
    return NULL;
}

void ThreadPool::run()
{
    while (true)
    {
        ThreadTask requestTask;
        {
            MutexLockGuard guard(this->mutex_);
            // 无任务 且未shutdown 则循环等待
            while (request_queue.empty() && !shutdown_)
            {
                condition_.wait();
            }

            if ((shutdown_ == immediate_mode) || (shutdown_ == graceful_mode && request_queue.empty()))
            {
                break;
            }

            // FIFO
            requestTask = request_queue.front();
            request_queue.pop_front();
        }
        requestTask.process(requestTask.arg);
    }
}

bool ThreadPool::isFull()
{
    return max_queue_size > 0 && request_queue.size() > max_queue_size;
}