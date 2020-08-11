//
// Created by Zhao Yingli on 2020/4/1 9:30 AM.
//

#include "webserver/Server.h"
#include "Util/Util.h"
#include "config/Config.h"

#include <iostream>
using namespace std;

std::string basePath = "."; //默认是程序当前目录

int main(int argc, char **argv)
{
    // 命令行解析
    Config config;
    config.parse_arg(argc, argv);

    if (config.daemon)
        daemon_run();

    //  输出配置信息
    {
        cout << "*******LC WebServer 配置信息*******" << endl;
        cout << "端口:\t" << config.PORT << endl;
        cout << "线程数:\t" << config.thread_num << endl;
        cout << "根目录:\t" << config.basePath.c_str() << endl;
    }

    // 根目录
    basePath = config.basePath;

    // 构造httpServer对象
    HttpServer httpServer(config.PORT, config.thread_num);
    
    // 开启线程池
    httpServer.start_threadpoll();
    
    // 监听
    httpServer.eventListen();
    
    // 运行
    httpServer.eventLoop();

    return 0;
}
