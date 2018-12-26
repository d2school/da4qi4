#ifndef DAQI_SESSION_HPP
#define DAQI_SESSION_HPP

#include <string>

#include "def/json_def.hpp"
#include "cookie.hpp"

namespace da4qi4
{

Json ToJson(Cookie const& cookie, Json const& data);
bool FromJson(Json const& node, Cookie& cookie, Json& data);

struct SessionOptions
{
    std::string name = "session_id";
    std::string prefix = "sid:";
    std::string domain;
    std::string path = "/";
    int max_age = 3600;
    Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_only;
    Cookie::Secure secure = Cookie::Secure::for_http_and_https;
    Cookie::SameSite samesite = Cookie::SameSite::none;
};

} //namespace da4qi4

#endif // DAQI_SESSION_HPP
