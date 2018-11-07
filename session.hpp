#ifndef DAQI_SESSION_HPP
#define DAQI_SESSION_HPP

#include <string>

#include "def/json_def.hpp"
#include "cookie.hpp"

namespace da4qi4
{

struct SessionData
{
    Cookie cookie;  //json name "c"
    Json data;      //json name "d"

    std::string const& GetID() const
    {
        return cookie.GetValue();
    }

    std::string ToString() const;
    bool FromString(std::string const& str);
};

struct SessionOptions
{
    std::string name = "session_id";
    std::string prefix = "sid:";
    std::string domain;
    std::string path = "/";
    size_t max_age = 3600;
    Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_only;
    Cookie::Secure secure = Cookie::Secure::for_http_and_https;
    Cookie::SameSite samesite = Cookie::SameSite::none;
};

} //namespace da4qi4

#endif // DAQI_SESSION_HPP
