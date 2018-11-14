#ifndef DAQI_INTERCEPTER_SESSION_REDIS_HPP
#define DAQI_INTERCEPTER_SESSION_REDIS_HPP

#include <string>

#include "def/json_def.hpp"
#include "intercepter.hpp"

#include "session.hpp"
#include "rediscli_pool.hpp"

namespace da4qi4
{
namespace Intercepter
{

struct SessionOnRedis
{
    static std::string const data_name;

    SessionOnRedis() = default;
    SessionOnRedis(std::string const& name,
                   std::string const& prefix,
                   int session_max_age_seconds)
    {
        _options.name = name;
        _options.prefix = prefix;
        _options.max_age = session_max_age_seconds;
    }

    SessionOnRedis(int session_max_age_seconds)
    {
        _options.max_age = session_max_age_seconds;
    }

    SessionOnRedis(SessionOptions const& options)
        : _options(options)
    {
    }

    SessionOnRedis(SessionOnRedis const&) = default;
    SessionOnRedis& operator = (SessionOnRedis const&) = default;

    SessionOnRedis& SetName(std::string const& name)
    {
        _options.name = name;
        return *this;
    }

    SessionOnRedis& SetPrefix(std::string const& prefix)
    {
        _options.prefix = prefix;
        return *this;
    }

    SessionOnRedis& SetMaxAge(size_t max_age)
    {
        _options.max_age = max_age;
        return *this;
    }

    SessionOnRedis& SetDomain(std::string const& domain)
    {
        _options.domain = domain;
        return *this;
    }

    SessionOnRedis& SetPath(std::string const& path)
    {
        _options.path = path;
        return *this;
    }

    SessionOnRedis& SetHttpOnly(Cookie::HttpOnly http_only)
    {
        _options.http_only = http_only;
        return *this;
    }

    SessionOnRedis& SetSecure(Cookie::Secure secure)
    {
        _options.secure = secure;
        return *this;
    }

    SessionOnRedis& SetSameSite(Cookie::SameSite samesite)
    {
        _options.samesite = samesite;
        return *this;
    }

    SessionOptions const& GetOptions() const
    {
        return _options;
    }

public:
    void operator()(Context ctx, On on) const;

private:
    void on_request(Context& ctx) const;
    void on_response(Context& ctx) const;

    Json create_new_session() const;
private:
    SessionOptions _options;
};


} //namespace Intercepter
} //namespace da4qi4

#endif // DAQI_INTERCEPTER_SESSION_REDIS_HPP
