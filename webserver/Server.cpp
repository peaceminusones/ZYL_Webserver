#include "Server.h"

#include "../Util/Util.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <string>
#include <functional>
#include <sys/epoll.h>
#include <vector>
#include <cstring>

char NOT_FOUND_PAGE[] = "<html>\n"
                        "<head><title>404 Not Found</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>404 Not Found</h1></center>\n"
                        "<hr><center>ZYL WebServer(Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char FORBIDDEN_PAGE[] = "<html>\n"
                        "<head><title>403 Forbidden</title></head>\n"
                        "<body bgcolor=\"white\">\n"
                        "<center><h1>403 Forbidden</h1></center>\n"
                        "<hr><center>ZYL WebServer(Linux)</center>\n"
                        "</body>\n"
                        "</html>";

char INDEX_PAGE[] = "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "    <title>Welcome to ZYL WebServer!</title>\n"
                    "    <style>\n"
                    "        body {\n"
                    "            width: 35em;\n"
                    "            margin: 0 auto;\n"
                    "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
                    "        }\n"
                    "    </style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<h1>Welcome to ZYL WebServer!</h1>\n"
                    "<p>If you see this page, the lc webserver is successfully installed and\n"
                    "    working. </p>\n"
                    "\n"
                    "<p>For online documentation and support please refer to\n"
                    "    <a href=\"https://github.com/MarvinLe/WebServer\">LC WebServer</a>.<br/>\n"
                    "\n"
                    "<p><em>Thank you for using LC WebServer.</em></p>\n"
                    "</body>\n"
                    "</html>";

extern std::string basePath;

// 开启线程池
void HttpServer::start_threadpoll()
{
    threadPool.start();
}

// 监听（HttpServer构造函数中已创建listenfd）
void HttpServer::eventListen()
{
    // epoll 创建内核事件表
    int epoll_fd = Epoll::init(1024);

    // 创建httpdata对象
    std::shared_ptr<HttpData> httpData(new HttpData());
    httpData->epoll_fd = epoll_fd;
    serverSocket.epoll_fd = epoll_fd;

    // 监听的事件类型（输入、ET）
    __uint32_t event = (EPOLLIN | EPOLLET);
    // 把服务器监听事件注册到内核时间表中
    Epoll::addfd(epoll_fd, serverSocket.listenfd, event, httpData);
}

// 运行
void HttpServer::eventLoop()
{
    while (true)
    {
        // 从就绪队列中取出就绪的事件
        // 如果没有，调用一直阻塞，直到有文件描述符进入就绪状态
        std::vector<std::shared_ptr<HttpData>> events = Epoll::poll(serverSocket, 1024, -1);
        // 将事件传递给线程池
        for (auto &req : events)
        {
            ThreadTask threadtask;
            threadtask.arg = req;
            threadtask.process = std::bind(&HttpServer::do_request, this, std::placeholders::_1);
            threadPool.append(threadtask);
        }
        // 处理定时器超时事件
        Epoll::timerManager.handle_expired_event();
    }
}

// 线程池中每个线程实际运行的程序
void HttpServer::do_request(std::shared_ptr<void> arg)
{
    // std::static_pointer_cast 将void类型的，强制转换为HttpData类型的指针
    std::shared_ptr<HttpData> sharedHttpData = std::static_pointer_cast<HttpData>(arg);

    // 读取缓冲区
    char buffer[BUFFERSIZE];
    bzero(buffer, BUFFERSIZE);

    // check_index 当前已经分析完了多少字节的客户数据
    // read_index 当前已经读取了多少字节的数据
    // start_line 行在buffer中的起始位置
    int check_index = 0, read_index = 0, start_line = 0;

    ssize_t recv_data;
    // HttpRequestParser::PARSE_REQUESTLINE 设置主状态机的初始状态，设置为“正在分析请求行”
    HttpRequestParser::PARSE_STATE parse_state = HttpRequestParser::PARSE_REQUESTLINE;
    // 循环读取客户数据并分析
    while (true)
    {
        // 由于是非阻塞IO，所以返回-1 也不一定是错误，还需判断error
        recv_data = recv(sharedHttpData->clientSocket_->fd, buffer + read_index, BUFFERSIZE - read_index, 0);
        if (recv_data == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                return; // FIXME 请求不完整该怎么办，继续加定时器吗？还是直接关闭
            }
            std::cout << "reading faild" << std::endl;
            return;
        }

        // todo 返回值为 0对端关闭, 这边也应该关闭定时器
        if (recv_data == 0)
        {
            std::cout << "connection closed by peer" << std::endl;
            break;
        }

        read_index += recv_data;
        // 分析目前已经获得的所有客户数据
        HttpRequestParser::HTTP_CODE retcode = HttpRequestParser::parse_content(
            buffer, check_index, read_index, parse_state, start_line, *sharedHttpData->request_);
        // 如果Http_Code为请求不完整，继续读取客户数据
        if (retcode == HttpRequestParser::NO_REQUEST)
        {
            continue;
        }

        // 如果Http_Code为获得了一个完整的客户请求
        if (retcode == HttpRequestParser::GET_REQUEST)
        {
            // mHeaders存储了{头域的枚举值：域值}
            // 检查 keep_alive选项
            auto it = sharedHttpData->request_->mHeaders.find(HttpRequest::Connection);
            if (it != sharedHttpData->request_->mHeaders.end())
            {
                if (it->second == "keep-alive")
                {
                    sharedHttpData->response_->setKeepAlive(true);
                    // timeout=20s
                    sharedHttpData->response_->addHeader("Keep-Alive", std::string("timeout=20"));
                }
                else
                {
                    sharedHttpData->response_->setKeepAlive(false);
                }
            }

            header(sharedHttpData);
            getMime(sharedHttpData);

            FileState fileState = static_file(sharedHttpData, basePath.c_str());
            send(sharedHttpData, fileState);
            // 如果是keep_alive else sharedHttpData将会自动析构释放clientSocket，从而关闭资源
            if (sharedHttpData->response_->keep_alive())
            {
                Epoll::modfd(sharedHttpData->epoll_fd, sharedHttpData->clientSocket_->fd, Epoll::DEFAULT_EVENTS, sharedHttpData);
                Epoll::timerManager.addTimer(sharedHttpData, TimerManager::DEFAULT_TIME_OUT);
            }
        }
        else // 其他情况表示发生错误
        {
            // todo Bad Request 应该关闭定时器(其实定时器已经关闭,在每接到一个新的数据时)
            std::cout << "Bad Request" << std::endl;
        }
    }
}

// http的版本类型
void HttpServer::header(std::shared_ptr<HttpData> httpData)
{
    if (httpData->request_->mVersion == HttpRequest::HTTP_11)
    {
        httpData->response_->setVersion(HttpRequest::HTTP_11);
    }
    else
    {
        httpData->response_->setVersion(HttpRequest::HTTP_10);
    }
    httpData->response_->addHeader("Server", "ZYL WebServer");
}

// 获取Mime 同时设置path到response
void HttpServer::getMime(std::shared_ptr<HttpData> httpData)
{
    std::string filepath = httpData->request_->mUri;
    std::string mime;

    // std::cout << "uri: " << filepath << std::endl;
    // 从后向前查找，返回在字符串中的位置
    if (filepath.rfind('?') != std::string::npos)
    {
        // 删除后面的字符
        filepath.erase(filepath.rfind('?'));
    }

    if (filepath.rfind('.') != std::string::npos)
    {
        // eg: ".html"
        mime = filepath.substr(filepath.rfind('.'));
    }

    decltype(Mime_map)::iterator it;
    if ((it = Mime_map.find(mime)) != Mime_map.end())
    {
        httpData->response_->setMime(it->second);
    }
    else
    {
        httpData->response_->setMime(Mime_map.find("default")->second);
    }
    httpData->response_->setFilePath(filepath);
}

// 设置文件状态与路径等
HttpServer::FileState HttpServer::static_file(std::shared_ptr<HttpData> httpData, const char *basepath)
{
    struct stat file_stat;
    char file[strlen(basepath) + strlen(httpData->response_->filePath().c_str()) + 1];
    strcpy(file, basepath);
    strcat(file, httpData->response_->filePath().c_str());

    // 文件不存在或者访问的是根目录
    if (httpData->response_->filePath() == "/" || stat(file, &file_stat) < 0)
    {
        // 设置Mime 为 html
        httpData->response_->setMime(MimeType("text/html"));
        if (httpData->response_->filePath() == "/")
        {
            httpData->response_->setStatusCode(HttpResponse::k200Ok);
            httpData->response_->setStatusMsg("OK");
        }
        else
        {
            httpData->response_->setStatusCode(HttpResponse::k404NotFound);
            httpData->response_->setStatusMsg("Not Found");
        }
        return FIlE_NOT_FOUND;
    }

    // 不是普通文件或无访问权限
    if (!S_ISREG(file_stat.st_mode))
    {
        // FIXME 设置Mime 为 html
        httpData->response_->setMime(MimeType("text/html"));
        httpData->response_->setStatusCode(HttpResponse::k403forbiden);
        httpData->response_->setStatusMsg("ForBidden");
        std::cout << "not normal file" << std::endl;
        return FILE_FORBIDDEN;
    }

    httpData->response_->setStatusCode(HttpResponse::k200Ok);
    httpData->response_->setStatusMsg("OK");
    httpData->response_->setFilePath(file);

    return FILE_OK;
}

void HttpServer::send(std::shared_ptr<HttpData> httpData, FileState fileState)
{
    char header[BUFFERSIZE];
    bzero(header, '\0');
    const char *internal_error = "Internal Error";
    struct stat file_stat;
    httpData->response_->appendBuffer(header);

    // 404
    if (fileState == FIlE_NOT_FOUND)
    {

        // 如果是 '/'开头就发送默认页
        if (httpData->response_->filePath() == std::string("/"))
        {
            // 现在使用测试页面
            sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(INDEX_PAGE));
            sprintf(header, "%s%s", header, INDEX_PAGE);
        }
        else
        {
            sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(NOT_FOUND_PAGE));
            sprintf(header, "%s%s", header, NOT_FOUND_PAGE);
        }
        ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
        return;
    }

    // 禁止访问
    if (fileState == FILE_FORBIDDEN)
    {
        sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(FORBIDDEN_PAGE));
        sprintf(header, "%s%s", header, FORBIDDEN_PAGE);
        ::send(httpData->clientSocket_->fd, header, (int)strlen(header), 0);
        return;
    }

    // 获取文件状态
    if (stat(httpData->response_->filePath().c_str(), &file_stat) < 0)
    {
        sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(internal_error));
        sprintf(header, "%s%s", header, internal_error);
        ::send(httpData->clientSocket_->fd, header, (int)strlen(header), 0);
        return;
    }

    int filefd = ::open(httpData->response_->filePath().c_str(), O_RDONLY);
    // 内部错误
    if (filefd < 0)
    {
        std::cout << "打开文件失败" << std::endl;
        sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(internal_error));
        sprintf(header, "%s%s", header, internal_error);
        ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
        close(filefd);
        return;
    }

    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)file_stat.st_size);
    ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
    void *mapbuf = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0);
    ::send(httpData->clientSocket_->fd, mapbuf, file_stat.st_size, 0);
    munmap(mapbuf, file_stat.st_size);
    close(filefd);
    return;
err:
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(internal_error));
    sprintf(header, "%s%s", header, internal_error);
    ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
    return;
}
