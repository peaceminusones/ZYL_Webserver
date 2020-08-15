#include "HttpRequest.h"
#include <unordered_map>

// HTTP请求的头部字段
std::unordered_map<std::string, HttpRequest::HTTP_HEADER> HttpRequest::header_map = {
    {"HOST", HttpRequest::Host},
    {"USER-AGENT", HttpRequest::User_Agent},
    {"CONNECTION", HttpRequest::Connection},
    {"ACCEPT-ENCODING", HttpRequest::Accept_Encoding},
    {"ACCEPT-LANGUAGE", HttpRequest::Accept_Language},
    {"ACCEPT", HttpRequest::Accept},
    {"CACHE-CONTROL", HttpRequest::Cache_Control},
    {"UPGRADE-INSECURE-REQUESTS", HttpRequest::Upgrade_Insecure_Requests}};

// 重载HttpRequest <<
std::ostream &operator<<(std::ostream &os, const HttpRequest &request)
{
    os << "method:" << request.mMethod << std::endl;
    os << "uri:" << request.mUri << std::endl;
    os << "version:" << request.mVersion << std::endl;
    //os << "content:" << request.mContent << std::endl;
    for (auto it = request.mHeaders.begin(); it != request.mHeaders.end(); it++)
    {
        os << it->first << ":" << it->second << std::endl;
    }
    return os;
}
