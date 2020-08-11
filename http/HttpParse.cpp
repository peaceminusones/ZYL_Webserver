//
// Created by marvinle on 2019/2/1 10:59 AM.
//

#include "HttpRequest.h"
#include "HttpParse.h"
#include "../Util/Util.h"

#include <string.h>
#include <iostream>
#include <algorithm>

std::unordered_map<std::string, HttpRequest::HTTP_HEADER> HttpRequest::header_map = {
    {"HOST", HttpRequest::Host},
    {"USER-AGENT", HttpRequest::User_Agent},
    {"CONNECTION", HttpRequest::Connection},
    {"ACCEPT-ENCODING", HttpRequest::Accept_Encoding},
    {"ACCEPT-LANGUAGE", HttpRequest::Accept_Language},
    {"ACCEPT", HttpRequest::Accept},
    {"CACHE-CONTROL", HttpRequest::Cache_Control},
    {"UPGRADE-INSECURE-REQUESTS", HttpRequest::Upgrade_Insecure_Requests}};

// 从状态机，用于解析出一行内容
// 解析一行内容, buffer[checked_index, read_index)
// check_index是需要分析的第一个字符， read_index已经读取数据末尾下一个字符

// 从状态机每次从缓冲区读取一行信息，直至读取到 \r\n 表示读取到一行，
// 同时将 \r\n 替换为 \0\0 便于主状态机读取该行，
// 然后再将行起始标志定位到下一行的起始位置。
HttpRequestParser::LINE_STATE
HttpRequestParser::parse_line(char *buffer, int &checked_index, int &read_index)
{
    // checked_index指向buffer（应用程序的读缓冲区）中当前正在分析的字节
    // read_index指向buffer中客户数据的尾部的下一个字节
    // buffer中第0～checked_index字节已经分析完毕
    // 第checked_index~(read_index-1)字节由下面的循环挨个分析
    char temp;
    for (; checked_index < read_index; checked_index++)
    {
        // 获得当前要分析的字节
        temp = buffer[checked_index];
        // 如果当前的字节是'\r'，说明可能读取到一个完整的行
        if (temp == CR)
        {
            // 如果‘\r’字符碰巧是目前buffer中的最后一个被读入的客户数据
            // 那么这次分析没有读到一个完整的行
            // 返回LINE_MORE表示还需要继续读取客户数据才能进一步分析
            if (checked_index + 1 == read_index)
                return LINE_MORE;
            // 如果下一个字符是‘\n’说明成功读到了一个完整的行
            else if (buffer[checked_index + 1] == LF)
            {
                buffer[checked_index++] = LINE_END;
                buffer[checked_index++] = LINE_END;
                return LINE_OK;
            }
            // 否则的话，说明客户发送的HTTP请求存在语法问题
            return LINE_BAD;
        }
        // 如果当前字符是‘\n’，即换行符，则说明可能读取到一个完整的行
        else if (temp == LF)
        {
            if (checked_index > 1 && buffer[checked_index - 1] == '\r')
            {
                buffer[checked_index--] = LINE_END;
                buffer[checked_index++] = LINE_END;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    // 如果所有内容都分析完毕也没遇到'\r'字符，则返回LINE_MORE
    // 表示还需要继续读取客户数据才能进一步分析
    return LINE_MORE;
}

// 解析请求行
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_requestline(char *line, PARSE_STATE &parse_state, HttpRequest &request)
{
    // 返回temp字符串中第一个出现空白字符或者'/t'字符的位置
    char *url = strpbrk(line, " \t");
    // 如果请求行中没有空白字符或者'\t'字符，则HTTP请求必有问题
    if (!url)
    {
        return BAD_REQUEST;
    }

    // 分割 method 和 url
    *url++ = '\0';

    char *method = line;
    // 分析请求方法是否为GET方法
    if (strcasecmp(method, "GET") == 0) // 忽略大小写比较字符串
    {
        request.mMethod = HttpRequest::GET;
    }
    else if (strcasecmp(method, "POST") == 0)
    {
        request.mMethod = HttpRequest::POST;
    }
    else if (strcasecmp(method, "PUT") == 0)
    {
        request.mMethod = HttpRequest::PUT;
    }
    else
    {
        return BAD_REQUEST;
    }

    // 清除前面多余的空格
    url += strspn(url, " \t");
    // 返回url字符串中第一个出现空白字符或者'/t'字符的位置
    char *version = strpbrk(url, " \t");
    if (!version)
    {
        return BAD_REQUEST;
    }

    // 分割url和version
    *version++ = '\0';
    version += strspn(version, " \t");

    // 分析协议版本字段的正确性
    // HTTP/1.1 后面可能还存在空白字符
    if (strncasecmp("HTTP/1.1", version, 8) == 0)
    {
        request.mVersion = HttpRequest::HTTP_11;
    }
    else if (strncasecmp("HTTP/1.0", version, 8) == 0)
    {
        request.mVersion = HttpRequest::HTTP_10;
    }
    else
    {
        return BAD_REQUEST;
    }

    // 检查url是否合法
    if (strncasecmp(url, "http://", 7) == 0)
    {
        url += 7;
        url = strchr(url, '/');
    }
    else if (strncasecmp(url, "/", 1) == 0)
    {
        PASS;
    }
    else
    {
        return BAD_REQUEST;
    }
    //若“http://“后面没有/字符或者url前七个字节不是 "http://"，则语法错误
    if (!url || *url != '/')
    {
        return BAD_REQUEST;
    }

    // std::cout << "THe request URL is: " << url << endl;

    request.mUri = std::string(url);
    // HTTP请求行处理完毕，状态转移到头部字段的分析
    parse_state = PARSE_HEADER;
    // 请求不完整，需要继续读取客户数据
    return NO_REQUEST;
}

// 分析头部字段，读取到空行说明头部正确
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_headers(char *line, PARSE_STATE &parse_state, HttpRequest &request)
{
    // 遇到空行，说明得到了一个正确的HTTP请求
    if (*line == '\0')
    {
        // 当请求方法为GET时，请求报文没有请求正文，
        // 也就是说解析完头部字段后该报文的解析过程就完成了
        if (request.mMethod == HttpRequest::GET)
        {
            return GET_REQUEST; // 获得了一个完整的客户请求
        }

        parse_state = PARSE_BODY; // 状态转移到解析body
        return NO_REQUEST;        // 请求不完整，需要继续请求客户端数据
    }

    // 把key和value分别提取出来
    char key[100], value[300];
    sscanf(line, "%[^:]:%[^:]", key, value);

    // decltype返回header_map的类型（unordered_map）
    decltype(HttpRequest::header_map)::iterator it;
    std::string key_s(key);

    // 把key值都转换成大写
    std::transform(key_s.begin(), key_s.end(), key_s.begin(), ::toupper);

    // 域值转为string类型
    std::string value_s(value);
    // cout << key_s << ": " << value_s << endl;

    // trim()除去两端的空格
    if ((it = HttpRequest::header_map.find(trim(key_s))) != (HttpRequest::header_map.end()))
    {
        // it->second: 是头域的枚举值，value_s是对应的域值的string类型
        request.mHeaders.insert(std::make_pair(it->second, trim(value_s)));
    }

    return NO_REQUEST;
}

// 解析body
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_body(char *body, HttpRequest &request)
{
    request.mContent = body;
    return GET_REQUEST;
}

// 分析HTTP请求的的入口函数，解析报文主函数
// http 请求入口
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_content(char *buffer, int &check_index, int &read_index,
                                 HttpRequestParser::PARSE_STATE &parse_state, int &start_line,
                                 HttpRequest &request)
{
    // 记录当前行的读取状态
    LINE_STATE line_state = LINE_OK;
    // 记录HTTP请求的处理结果
    HTTP_CODE retcode = NO_REQUEST;
    // 主状态机，用于从buffer中取出所有完整的行
    while ((line_state = parse_line(buffer, check_index, read_index)) == LINE_OK)
    {
        // start_line是这一行在buffer中的起始位置
        char *temp = buffer + start_line; // 这一行在buffer中的起始位置
        start_line = check_index;         // 记录下一行的起始位置

        // parse_state 记录主状态机当前的状态
        switch (parse_state)
        {
        // 第一个状态，分析请求行
        case PARSE_REQUESTLINE:
        {
            retcode = parse_requestline(temp, parse_state, request);
            if (retcode == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }

        // 第二个状态，分析头部字段
        case PARSE_HEADER:
        {
            retcode = parse_headers(temp, parse_state, request);
            if (retcode == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if (retcode == GET_REQUEST)
            {
                return GET_REQUEST;
            }
            break;
        }

        // 第三个状态，分析主体
        case PARSE_BODY:
        {
            retcode = parse_body(temp, request);
            if (retcode == GET_REQUEST)
            {
                return GET_REQUEST;
            }
            return BAD_REQUEST;
        }

        // 默认状态：服务器内部错误
        default:
            return INTERNAL_ERROR;
        }
    }

    // 若没有读取到一个完整的行，则表示还需要继续读取客户数据才能进一步分析
    if (line_state == LINE_MORE)
    {
        return NO_REQUEST;
    }
    else
    {
        // 表示客户请求有语法错误
        return BAD_REQUEST;
    }
}
