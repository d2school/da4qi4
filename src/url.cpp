#include "daqi/url.hpp"

#include <cassert>

#include "daqi/utilities/http_utilities.hpp"

#include "llhttp/helper/http_url_def.h"

namespace da4qi4
{

namespace
{
bool get_url_part_value(unsigned int url_part_flag,  UrlBase& url, std::string&& value)
{
    switch (url_part_flag)
    {
        case UF_SCHEMA :
            url.schema = std::move(value);
            break;

        case UF_HOST :
            url.host = std::move(value);
            break;

        case UF_PORT:
            break; //skip, but return true;

        case UF_PATH :
            url.path = std::move(value);
            break;

        case UF_QUERY :
            url.query = std::move(value);
            break;

        case UF_FRAGMENT :
            url.fragment = std::move(value);
            break;

        case UF_USERINFO :
            url.userinfo = std::move(value);
            break;

        default:
            return false;
    }

    return true;
}
}

bool UrlBase::Parse(std::string&& url_value)
{
    http_parser_url r;
    http_parser_url_init(&r);
    int err = http_parser_parse_url(url_value.c_str(), url_value.length(), 0, &r);

    if (0 == err)
    {
        port = r.port;

        for (unsigned int i = 0; i < UF_MAX; ++i)
        {
            if ((r.field_set & (1 << i)) == 0)
            {
                continue;
            }

            std::string part(url_value.c_str() + r.field_data[i].off, r.field_data[i].len);
            get_url_part_value(i, *this, std::move(part));
        }

        if (!query.empty())
        {
            parameters = Utilities::ParseQueryParameters(query);
        }
    }

    full = std::move(url_value);

    return !err;
}

void UrlUnderApp::UnderApplication(std::string const& app_url_root)
{
    assert(Utilities::iStartsWith(full, app_url_root));
    assert(Utilities::iStartsWith(path, app_url_root));

    full_under_app = full.substr(app_url_root.size());
    path_under_app = path.substr(app_url_root.size());
}

UrlBase FromUrlUnderApp(UrlUnderApp&& src)
{
    UrlBase dst;
    dst.port = src.port;

#define MOVE_FROM_APP_URL(D, I, S) D.I = std::move(S.I)

    MOVE_FROM_APP_URL(dst, full, src);
    MOVE_FROM_APP_URL(dst, schema, src);
    MOVE_FROM_APP_URL(dst, host, src);
    MOVE_FROM_APP_URL(dst, path, src);
    MOVE_FROM_APP_URL(dst, query, src);
    MOVE_FROM_APP_URL(dst, fragment, src);
    MOVE_FROM_APP_URL(dst, userinfo, src);
    MOVE_FROM_APP_URL(dst, parameters, src);

#undef MOVE_FROM_APP_URL

    return dst;
}


std::string JoinUrlPath(std::string const& app_root, std::string const& path)
{
    if (!path.empty() && !app_root.empty())
    {
        if (*app_root.rbegin() == '/' && *path.begin() == '/')
        {
            return app_root + path.substr(1, path.size() - 1);
        }

        if (*app_root.rbegin() != '/' && *path.begin() != '/')
        {
            return app_root + "/" + path;
        }
    }

    return app_root + path;
}

std::string MakesureFullUrlPath(std::string const& path, UrlFlag flag, const std::string& app_root)
{
    return (flag == UrlFlag::url_full_path) ? path : JoinUrlPath(app_root, path);
}


} // namespace da4qi4
