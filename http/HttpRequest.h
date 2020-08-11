//
// Created by marvinle on 2019/2/26 6:36 PM.
//

//#ifndef WEBSERVER_HTTPREQUEST_H
//#define WEBSERVER_HTTPREQUEST_H
#pragma once

#include <iostream>
#include <unordered_map>

class HttpRequest;

std::ostream &operator<<(std::ostream &, const HttpRequest &);

// HTTP头域包括通用头，请求头，响应头，实体头
// 每个头域由一个（域名+冒号+域值）三个部分组成
// 在HttpRequest类中，存储在header_map中{string: enum}
struct HttpRequest
{
    // 重载 HttpRequest <<
    friend std::ostream &operator<<(std::ostream &, const HttpRequest &);

    enum HTTP_VERSION
    {
        HTTP_10 = 0,
        HTTP_11,
        VERSION_NOT_SUPPORT
    };

    enum HTTP_METHOD
    {
        GET = 0,
        POST,
        PUT,
        DELETE,
        METHOD_NOT_SUPPORT
    };

    enum HTTP_HEADER
    {
        Host = 0,
        User_Agent,
        Connection,
        Accept_Encoding,
        Accept_Language,
        Accept,
        Cache_Control,
        Upgrade_Insecure_Requests
    };

    // 计算枚举类的哈希值
    // 然后它就可以作为unordered_map第三个模板参数
    struct EnumClassHash
    {
        template <typename T>
        std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }
    };

    // 构造函数
    HttpRequest(std::string url = std::string(""), HTTP_METHOD method = METHOD_NOT_SUPPORT,
                HTTP_VERSION version = VERSION_NOT_SUPPORT) : mMethod(method), mVersion(version), mUri(url), mContent(nullptr),
                                                              mHeaders(std::unordered_map<HTTP_HEADER, std::string, EnumClassHash>()){};

    HTTP_VERSION mVersion; //HTTP版本号
    HTTP_METHOD mMethod;   // HTTP的请求方法
    // {HTTP_HEADER的字符串形式：枚举值}
    static std::unordered_map<std::string, HTTP_HEADER> header_map; // HTTP头部信息

    std::string mUri; // 请求链接
    char *mContent;   // 消息正文

    // 把header_map中{string:enum}，从string索引转换成枚举类的值作为键值！
    // EnumClassHash是枚举值的哈希值
    // string 类型存储的是头域对应的域值
    std::unordered_map<HTTP_HEADER, std::string, EnumClassHash> mHeaders;
};

//#endif //WEBSERVER_HTTPREQUEST_H
