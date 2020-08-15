#include "ThreadPool.h"
#include <iostream>
#include <assert.h>
#include <sys/prctl.h>

ThreadPool::ThreadPool(int thread_s, int max_queue_s) : thread_size(thread_s), max_queue_size(max_queue_s)
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
    m_threads.reserve(thread_size);
}

ThreadPool::~ThreadPool()
{
    if (!shutdown_)
        shutdown(1);
}

// 开启线程池
void ThreadPool::start()
{
    assert(m_threads.empty());
    shutdown_ = 0;

    for (int i = 0; i < thread_size; i++)
    {
        m_threads.push_back(std::thread(&ThreadPool::worker, this));
    }
}

// 关闭线程池
void ThreadPool::shutdown(bool graceful)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (shutdown_)
        {
            std::cout << "has shutdown" << std::endl;
        }

        shutdown_ = graceful ? graceful_mode : immediate_mode;
        m_cond.notify_all();
    }

    for (int i = 0; i < m_threads.size(); i++)
    {
        if (!m_threads[i].joinable())
        {
            std::cout << "pthread_join error" << std::endl;
        }

        m_threads[i].join();
    }
}

// 添加任务函数
// 添加一个任务到线程池
bool ThreadPool::append(const ThreadTask &task)
{
    if (shutdown_)
    {
        std::cout << "ThreadPool has shutdown" << std::endl;
        return false;
    }

    if (m_threads.empty())
    {
        task.process(task.arg);
        return false;
    }
    else
    {
        // 线程池被多个线程共享，操作前需要加锁
        std::unique_lock<std::mutex> lock(m_mutex);
        // 判断任务队列是否满了，满了则添加失败
        if (isFull())
        {
            std::cout << max_queue_size;
            std::cout << "ThreadPool too many requests" << std::endl;
            return false;
        }

        // 把任务添加到任务队列中
        m_request_queue.push_back(task);

        // 唤醒线程池中等待任务的线程
        m_cond.notify_all();

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
    while (!shutdown_)
    {
        ThreadTask requestTask;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            // 无任务 且未shutdown 则循环等待
            while (m_request_queue.empty())
            {
                m_cond.wait(lock);
            }

            if ((shutdown_ == immediate_mode) || (shutdown_ == graceful_mode && m_request_queue.empty()))
            {
                break;
            }

            // FIFO
            requestTask = m_request_queue.front();
            m_request_queue.pop_front();
        }
        requestTask.process(requestTask.arg);
    }
}

bool ThreadPool::isFull()
{
    return max_queue_size > 0 && m_request_queue.size() > max_queue_size;
}