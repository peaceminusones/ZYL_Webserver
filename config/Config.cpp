#include "Config.h"

Config::Config()
{
    // 端口号,默认7244
    PORT = 7244;

    // 日志写入方式，默认同步
    LOGWrite = 0;

    // 触发组合模式,默认listenfd LT + connfd LT
    TRIGMode = 0;

    // listenfd触发模式，默认LT
    LISTENTrigmode = 0;

    // connfd触发模式，默认LT
    CONNTrigmode = 0;

    // 优雅关闭链接，默认不使用
    OPT_LINGER = 0;

    // 数据库连接池数量,默认8
    sql_num = 8;

    // 线程池内的线程数量,默认8
    thread_num = 8;

    // 关闭日志,默认不关闭
    close_log = 0;

    // 并发模型,默认是proactor
    actor_model = 0;

    // 根目录
    basePath = ".";

    // 是否开启守护进程
    bool daemon = false;
}

void Config::parse_arg(int argc, char *argv[])
{
    int opt;
    const char *str = "p:l:m:o:s:t:c:a:r:d";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            PORT = atoi(optarg);
            break;
        }
        case 'l':
        {
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm':
        {
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o':
        {
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':
        {
            sql_num = atoi(optarg);
            break;
        }
        case 't':
        {
            thread_num = atoi(optarg);
            break;
        }
        case 'c':
        {
            close_log = atoi(optarg);
            break;
        }
        case 'a':
        {
            actor_model = atoi(optarg);
            break;
        }
        case 'r':
        {
            char tempPath[256];
            int ret = check_base_path(optarg);
            if (ret == -1)
            {
                cout << "Warning: " << optarg << "不存在或不可访问, 将使用当前目录作为网站根目录" << endl;
                // 获得当前目录地址
                if (getcwd(tempPath, sizeof(tempPath)) == NULL)
                {
                    perror("getcwd error");
                    basePath = ".";
                }
                else
                {
                    basePath = tempPath;
                }
                break;
            }
            // 如果最后有个‘/’，换成字符串结束符
            if (optarg[strlen(optarg) - 1] == '/')
            {
                optarg[strlen(optarg) - 1] = '\0';
            }

            basePath = optarg;
            break;
        }
        case 'd':
        {
            daemon = true;
            break;
        }
        default:
            break;
        }
    }
}