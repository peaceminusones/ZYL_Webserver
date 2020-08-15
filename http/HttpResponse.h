//#ifndef WEBSERVER_HTTPRESPONSE_H
//#define WEBSERVER_HTTPRESPONSE_H
#pragma once

#include "HttpRequest.h"

#include <string>
#include <unordered_map>
#include <memory>

struct MimeType
{
    MimeType(const std::string &str) : type(str){};

    MimeType(const char *str) : type(str){};

    std::string type;
};

extern std::unordered_map<std::string, MimeType> Mime_map;

// HTTP应答
// 第一行是状态行，包括http版本信息、状态码、状态信息
// 后面是HTTP应答的头部字段，与HTTP请求中的头部字段相同
// 其中content-type 表示目标文档的MIME类型
class HttpResponse
{

public:
    // http状态码
    enum HttpStatusCode
    {
        Unknow,
        k200Ok = 200,
        k403forbiden = 403,
        k404NotFound = 404,
        k500InetError = 500
    };

    explicit HttpResponse(bool mkeep = true)
        : mStatusCode(Unknow), keep_alive_(mkeep), mMime("text/html"), mBody(nullptr),
          mVersion(HttpRequest::HTTP_11) {}

    void setStatusCode(HttpStatusCode code)
    {
        mStatusCode = code;
    }

    void setBody(const char *buf)
    {
        mBody = buf;
    }

    void setContentLength(int len)
    {
        mContentLength = len;
    }

    void setVersion(const HttpRequest::HTTP_VERSION &version)
    {
        mVersion = version;
    }

    void setStatusMsg(const std::string &msg)
    {
        mStatusMsg = msg;
    }

    void setFilePath(const std::string &path)
    {
        mFilePath = path;
    }

    void setMime(const MimeType &mime)
    {
        mMime = mime;
    }

    void setKeepAlive(bool isalive)
    {
        keep_alive_ = isalive;
    }

    void addHeader(const std::string &key, const std::string &value)
    {
        mHeaders[key] = value;
    }

    bool keep_alive() const
    {
        return keep_alive_;
    }

    const HttpRequest::HTTP_VERSION version() const
    {
        return mVersion;
    }

    const std::string &filePath() const
    {
        return mFilePath;
    }

    HttpStatusCode statusCode() const
    {
        return mStatusCode;
    }

    const std::string &statusMsg() const
    {
        return mStatusMsg;
    }

    void appendBuffer(char *) const;

    ~HttpResponse()
    {
        if (mBody != nullptr)
            delete[] mBody;
    }

private:
    HttpStatusCode mStatusCode;
    HttpRequest::HTTP_VERSION mVersion;
    std::string mStatusMsg;
    bool keep_alive_;
    MimeType mMime;
    const char *mBody;
    int mContentLength;
    std::string mFilePath;
    std::unordered_map<std::string, std::string> mHeaders;
};

//#endif //WEBSERVER_HTTPRESPONSE_H
