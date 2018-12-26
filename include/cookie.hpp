#ifndef DAQI_COOKIE_HPP
#define DAQI_COOKIE_HPP

#include <ctime>

#include <iostream>
#include <string>
#include <limits>

#include "def/json_def.hpp"

namespace da4qi4
{
struct Cookie
{
    enum class HttpOnly {for_http_and_js = 0, for_http_only = 1};
    enum class Secure {for_http_and_https = 0, for_https_only = 1};
    enum class SameSite {none = 0, lax, strict};

    Cookie() = default;
    Cookie(Cookie const&) = default;
    Cookie(Cookie&& o)
    {
        _old_version = o._old_version;

        _name = std::move(o._name);
        _value = std::move(o._value);
        _domain = std::move(o._domain);
        _path = std::move(o._path);

        _max_age = o.expires_after_brower_close;

        _http_only = o._http_only;
        _secure = o._secure;

        _samesite = o._samesite;
    }

    Cookie& operator = (Cookie const& o) = default;
    Cookie& operator = (Cookie&& o) = default;

    Cookie(std::string const& name, std::string const& value)
        : _name(name), _value(value)
    {}

    Cookie(std::string const& name, std::string const& value, std::string const& domain)
        : _name(name), _value(value), _domain(domain)
    {}

    Cookie(std::string const& name, std::string const& value
           , std::string const& domain, std::string const& path)
        : _name(name), _value(value), _domain(domain), _path(path)
    {}

    std::string const& GetName() const
    {
        return _name;
    }

    std::string const& GetDomain() const
    {
        return _domain;
    }

    std::string const& GetPath() const
    {
        return _path;
    }

    std::string const& GetValue() const
    {
        return _value;
    }

    Cookie& SetName(std::string const& name)
    {
        _name = name;
        return *this;
    }

    Cookie& SetValue(std::string const& value)
    {
        _name = value;
        return *this;
    }

    Cookie& SetDomain(std::string const& domain)
    {
        _domain = domain;
        return *this;
    }

    Cookie& SetPath(std::string const& path)
    {
        _path = path;
        return *this;
    }

    Cookie& ApplyHttpVersion(unsigned short http_version_major, unsigned short http_version_minor)
    {
        _old_version = ((http_version_major < 1)
                        || (http_version_major == 1 && http_version_minor == 0));
        return *this;
    }

    Cookie& SetHttpOnly(HttpOnly only)
    {
        _http_only = only == HttpOnly::for_http_only;
        return *this;
    }

    bool IsHttpOnly() const
    {
        return _http_only;
    }

    Cookie& SetMaxAge(int seconds)
    {
        _max_age = (seconds >= 0) ? seconds : 0;
        return *this;
    }

    int GetMaxAge() const
    {
        return _max_age;
    }

    Cookie& SetExpires(std::time_t a_time_point);

    Cookie& SetExpiredAfterBrowerClose()
    {
        _max_age = expires_after_brower_close;
        return *this;
    }

    bool IsExpiredAfterBrowerClose() const
    {
        return _max_age == expires_after_brower_close;
    }

    Cookie& SetExpiredImmediately()
    {
        _max_age = expires_immediately;
        return *this;
    }

    bool IsExpiredImmediately() const
    {
        return _max_age == expires_immediately;
    }

    bool IsOldVersion() const
    {
        return _old_version;
    }

    bool IsSecure() const
    {
        return _secure;
    }

    Cookie& SetSecure(Secure secure)
    {
        _secure = (secure == Secure::for_https_only);
        return *this;
    }

    SameSite GetSameSite() const
    {
        return _samesite;
    }

    Cookie& SetSameSite(SameSite ss)
    {
        _samesite = ss;
        return *this;
    }

    void ClearValue()
    {
        _value.clear();
    }

private:
    enum
    {
        expires_after_brower_close = std::numeric_limits<int>::min()
        , expires_immediately
    };

    bool _old_version = false;

    std::string _name;
    std::string _value;
    std::string _domain;
    std::string _path;
    int _max_age = expires_after_brower_close;

    bool _http_only = false;
    bool _secure = false;

    SameSite _samesite = SameSite::none;

    friend std::ostream& operator << (std::ostream&, Cookie const&);
    friend void to_json(Json&,  Cookie const&);
    friend void from_json(Json const&, Cookie&);
};

std::ostream& operator << (std::ostream& os, Cookie const& c);
void to_json(Json& j,  Cookie const& c);
void from_json(Json const& j, Cookie& c);


} //namespace da4qi4
#endif // DAQI_COOKIE_HPP
