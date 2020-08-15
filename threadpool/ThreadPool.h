//#ifndef WEBSERVER_THREADPOLL_H
//#define WEBSERVER_THREADPOLL_H

#pragma once

#include <vector>
#include <list>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "../Util/noncopyable.h"

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

/*
 * 主线程往工作队列中插入任务，工作线程通过竞争来取得任务并执行它
*/

// 线程池
class ThreadPool : public noncopyable
{
public:
    // 构造函数
    ThreadPool(int thread_s, int max_queue_s);
    // 析构函数
    ~ThreadPool();

    void start();                 // 开启线程池
    void shutdown(bool graceful); // 关闭线程池
    // 添加一个任务(arg:参数 fun:函数指针)
    bool append(const ThreadTask&);

    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void *worker(void *args); 
    void run();                      // 线程工作函数
    bool isFull();                   // 任务队列是否已满

private:
    // 禁止赋值拷贝
    ThreadPool(const ThreadPool&);
    const ThreadPool& operator=(const ThreadPool&);

    std::mutex m_mutex;             // 保护请求队列的互斥锁
    std::condition_variable m_cond; // 是否有任务要处理的

    // 线程池属性
    int thread_size;                     // 线程池中的线程数
    int max_queue_size;                  // 请求队列中允许的最大请求数
    int shutdown_;                       // 判断线程池是否关闭，且关闭模式是什么
    // int idle;                         // 线程池中空闲线程数
    std::vector<std::thread> m_threads;   // 线程池的数组
    std::list<ThreadTask> m_request_queue; // 任务链表
};

//#endif //WEBSERVER_THREADPOLL_H
