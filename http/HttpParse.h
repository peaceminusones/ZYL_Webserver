//
// Created by marvinle on 2019/2/1 10:57 AM.
//

//#ifndef WEBSERVER_HTTPPARSE_H
//#define WEBSERVER_HTTPPARSE_H

#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

#define CR '\r'
#define LF '\n'
#define LINE_END '\0'
#define PASS

class HttpRequest;

std::ostream &operator<<(std::ostream &, const HttpRequest &);

class HttpRequestParser
{
public:
    enum LINE_STATE // 从状态机的三种可能状态
    {
        LINE_OK = 0, // 读取到一个完整的行
        LINE_BAD,    // 行出错
        LINE_MORE    // 行数据尚且不完整
    };

    enum PARSE_STATE // 当前主状态机的状态
    {
        PARSE_REQUESTLINE = 0, // 当前正在分析请求行
        PARSE_HEADER,          // 当前正在分析头部字段
        PARSE_BODY             // 当前正在分析正文字段
    };

    enum HTTP_CODE // 服务器端处理HTTP请求的结果
    {
        NO_REQUEST,        // 请求不完整，需要继续读取客户数据
        GET_REQUEST,       // 获得了一个完整的客户请求
        BAD_REQUEST,       // 表示客户请求有语法错误
        FORBIDDEN_REQUEST, // 客户对资源没有足够的访问权限
        INTERNAL_ERROR,    // 表示服务器内部错误
        CLOSED_CONNECTION  // 表示客户端已经关闭链接
    };

    // 从状态机，用于解析出一行内容
    static LINE_STATE parse_line(char *buffer, int &checked_index, int &read_index);
    // 分析请求行
    static HTTP_CODE parse_requestline(char *line, PARSE_STATE &parse_state, HttpRequest &request);
    // 分析请求头部
    static HTTP_CODE parse_headers(char *line, PARSE_STATE &parse_state, HttpRequest &request);
    // 分析请求内容
    static HTTP_CODE parse_body(char *body, HttpRequest &request);

    static HTTP_CODE parse_content(char *buffer, int &check_index, int &read_index, PARSE_STATE &parse_state, int &start_line,
                                   HttpRequest &request);
};

//#endif //WEBSERVER_HTTPPARSE_H
