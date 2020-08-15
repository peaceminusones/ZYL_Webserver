//#ifndef WEBSERVER_HTTPREQUEST_H
//#define WEBSERVER_HTTPREQUEST_H
#pragma once

#include <iostream>
#include <unordered_map>

class HttpRequest;

// HTTP请求包括请求行、头部字段、消息体
// 每个头部字段由一个（字段名成+冒号+字段值）三个部分组成
// 在HttpRequest类中，将字段存储在header_map中{string: enum}
struct HttpRequest
{
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

    std::string mUri; // 请求链接
    char *mContent;   // 消息正文

    // {头部字段的名称：相应的枚举值}
    static std::unordered_map<std::string, HTTP_HEADER> header_map; // HTTP头部信息
    // 把header_map中{string:enum}，从string索引转换成枚举类的值作为键值！
    // EnumClassHash是枚举值的哈希值
    // {相应的枚举值：头部字段的名称}
    std::unordered_map<HTTP_HEADER, std::string, EnumClassHash> mHeaders; // HTTP头部字段信息
};

//#endif //WEBSERVER_HTTPREQUEST_H
