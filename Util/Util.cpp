//
// Created by marvinle on 2019/2/1 12:18 PM.
//

#include "Util.h"

#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h> // sigaction
#include <cstring>
#include <sys/stat.h>
#include <sys/socket.h>
using namespace std;

string &ltrim(string &str)
{
    if (str.empty())
    {
        return str;
    }

    str.erase(0, str.find_first_not_of(" \t"));
    return str;
}

string &rtrim(string &str)
{
    if (str.empty())
    {
        return str;
    }
    str.erase(str.find_last_not_of(" \t") + 1);
    return str;
}

// 除去两端空格
string &trim(string &str)
{
    if (str.empty())
    {
        return str;
    }

    ltrim(str);
    rtrim(str);
    return str;
}

// socket设置成非阻塞方式
int setnonblocking(int fd)
{
    // F_GETFL: 获取文件的flags，即open函数的第二个参数
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    // F_SETFL: 增加文件的某个flags，比如文件是阻塞的，设置成非阻塞的
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void handle_for_sigpipe()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    // sa_handler:是一个函数指针，指向一个信号处理函数
    // 执行该语句之后，中断键对该程序无效
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL))
        return;
}

// struct stat这个结构体是用来描述一个linux系统文件系统中的文件属性的结构
int check_base_path(char *basePath)
{
    struct stat file;
    if (stat(basePath, &file) == -1)
    {
        return -1;
    }
    // 不是目录 或者不可访问
    if (!S_ISDIR(file.st_mode) || access(basePath, R_OK) == -1)
    {
        return -1;
    }
    return 0;
}

void setReusePort(int fd)
{
    int opt = 1;
    // 一般来说，一个端口释放后会等待两分钟之后才能再被使用
    // SO_REUSEADDR是让端口释放后立即就可以被再次使用
    // 相当于没有了TIME_WAIT阶段
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
}

void daemon_run()
{
    int pid;
    signal(SIGCHLD, SIG_IGN);
    //1）在父进程中，fork返回新创建子进程的进程ID；
    //2）在子进程中，fork返回0；
    //3）如果出现错误，fork返回一个负值；
    pid = fork();
    if (pid < 0)
    {
        std::cout << "fork error" << std::endl;
        exit(-1);
    }
    //父进程退出，子进程独立运行
    else if (pid > 0)
    {
        exit(0);
    }
    //之前parent和child运行在同一个session里,parent是会话（session）的领头进程,
    //parent进程作为会话的领头进程，如果exit结束执行的话，那么子进程会成为孤儿进程，并被init收养。
    //执行setsid()之后,child将重新获得一个新的会话(session)id。
    //这时parent退出之后,将不会影响到child了。
    setsid();
    int fd;
    fd = open("/dev/null", O_RDWR, 0);
    if (fd != -1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if (fd > 2)
        close(fd);
}