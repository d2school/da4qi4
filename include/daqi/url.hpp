#ifndef DAQI_URL_HPP
#define DAQI_URL_HPP

#include <string>

#include "daqi/def/def.hpp"

namespace da4qi4
{

struct Url
{
    std::string full;

    std::string schema;
    std::string host;

    unsigned short port;

    std::string path;
    std::string query;
    std::string fragment;
    std::string userinfo;

    UrlParameters parameters;

    bool Parse(std::string&& url_value);

    void Clear()
    {
        full.clear();
        schema.clear();
        host.clear();
        port = 0;
        path.clear();
        query.clear();
        fragment.clear();
        userinfo.clear();
        parameters.clear();
    }
};

struct UrlUnderApp : public Url
{
    std::string full_under_app;
    std::string path_under_app;

    void UnderApplication(std::string const& app_url_root);
};

Url FromUrlUnderApp(UrlUnderApp&& src);

} //namespace da4qi4

#endif // DAQI_URL_HPP
