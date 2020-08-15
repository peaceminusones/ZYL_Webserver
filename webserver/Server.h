//#ifndef WEBSERVER_SERVER_H
//#define WEBSERVER_SERVER_H
#pragma once

#include "../socket/Socket.h"
#include "../http/HttpParse.h"
#include "../http/HttpResponse.h"
#include "../http/HttpData.h"
#include "../http/HttpRequest.h"
#include "../Epoll/Epoll.h"

#include "../threadpool/ThreadPool.h"
#include <memory>

#define BUFFERSIZE 2048

class HttpServer
{
public:
    enum FileState
    {
        FILE_OK,
        FIlE_NOT_FOUND,
        FILE_FORBIDDEN
    };

public:
    HttpServer(int port = 80, int thread_num = 8, const char *ip = nullptr) : serverSocket(port, ip), threadPool(thread_num, MAX_QUEUE_SIZE)
    {
        // 绑定ip地址、端口等信息到socket上
        serverSocket.bind();
        // 监听socket
        serverSocket.listen();
    }

    void start_threadpoll();
    void eventListen();
    void eventLoop();
    void do_request(std::shared_ptr<void> arg);

private:
    void header(std::shared_ptr<HttpData>);
    void send(std::shared_ptr<HttpData>, FileState);
    void getMime(std::shared_ptr<HttpData>);
    FileState static_file(std::shared_ptr<HttpData>, const char *);
    void handleIndex();

    ServerSocket serverSocket; // 构造函数中创建服务器监听socket，获得fd
    ThreadPool threadPool;     // 构造函数中创建线程池
};

// class HttpServer
// {
// public:
//     HttpServer();
//     ~HttpServer();

//     void init(int port, int thread_num, const char *ip = nullptr);

//     void thread_pool();
//     void eventListen();
//     void eventLoop();
//     void timer(int connfd, struct sockaddr_in client_address);
//     void adjust_timer(TimerNode *timer);
//     void deal_timer();
//     bool deal_clientdata();
//     bool dealwithsignal();
//     void dealwithread(int connfd);
//     void dealwithwrite(int connfd);
// };

//#endif //WEBSERVER_SERVER_H