#ifndef DAQI_RESPONSE_HPP
#define DAQI_RESPONSE_HPP

#include <iostream>
#include <functional>
#include <vector>
#include <limits>

#include <mutex>

#include "http-parser/http_parser.h"

#include "def/def.hpp"
#include "def/boost_def.hpp"

namespace da4qi4
{

std::string const&  EmptyBody();

struct Cookie
{
    enum class HttpOnly {for_http_and_js = 0, for_http_only = 1};
    enum class Secure {for_http_and_https = 0, for_https_only = 1};
    enum class SameSite {none = 0, lax, strict};

    Cookie() = default;
    Cookie(Cookie const&) = default;

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

    friend std::ostream& operator << (std::ostream& os, Cookie const& c);
};

std::ostream& operator << (std::ostream& os, Cookie const& c);

struct ChunkedBodies
{
    void PushBack(std::string const& body, bool is_last);
    std::string PopFront(bool& is_last);
    void Clear();

private:
    std::list<std::pair<std::string, bool>> _chunked_bodies;
    std::mutex _m;
};

class Response
{
public:
    Response();
    Response(http_status code)
        : _status_code(code)
    {}

    int GetStatusCode() const
    {
        return _status_code ;
    }

    void GetVersion(unsigned short& major, unsigned short& minor) const
    {
        major = _version_major;
        minor = _version_minor;
    }

    std::pair<unsigned short, unsigned short> GetVersion() const
    {
        return {_version_major, _version_minor};
    }

    std::string const& GetCharset() const
    {
        return _charset;
    }

    enum ContentTypePart {without_chartset = 0, with_chartset = 1};
    std::string GetContentType(ContentTypePart part = with_chartset) const;
    std::string const& GetContentEncoding() const
    {
        return GetHeader("Content-Encoding");
    }

    ICHeaders const& GetHeaders() const
    {
        return _headers;
    }

    std::string const& GetBody() const
    {
        return _body;
    }

    std::string PopChunkedBody(bool& is_last)
    {
        return _chunked_bodies.PopFront(is_last);
    }

    void PushChunkedBody(std::string const& body, bool is_last)
    {
        _chunked_bodies.PushBack(body, is_last);
    }

    bool IsExistsHeader(std::string const& field) const;
    std::string const& GetHeader(std::string const& field) const;
    OptionalStringRefConst TryGetHeader(std::string const& field) const;

    void SetStatusCode(http_status code)
    {
        _status_code = code;
    }

    void SetVersion(unsigned short major, unsigned short minor)
    {
        _version_major = major;
        _version_minor = minor;
    }

    void SetBody(std::string&& body)
    {
        _body = std::move(body);
    }

    void SetBody(std::string const& body)
    {
        _body = body;
    }

    void SetCharset(std::string const& charset);
    void SetContentType(std::string const& content_type);
    void SetContentType(std::string const& content_type
                        , std::string const& content_charset);

    void SetContentEncoding(std::string const& encoding)
    {
        AppendHeader("Content-Encoding", encoding);
    }

    void AppendHeader(std::string const& field, std::string const& value);

    bool IsRedirected() const
    {
        return IsExistsHeader("Location");
    }

    void SetLocation(std::string const& dst_location)
    {
        AppendHeader("Location", dst_location);
    }

    void SetExpires(std::time_t time_point)
    {
        AppendHeader("Expires", Utilities::GMTFormatTime(time_point));
    }

    void SetNoCache()
    {
        AppendHeader("Cache-Control", "no-cache");
    }
    void SetPublicCache(int max_age_seconds = 0)
    {
        AppendHeader("Cache-Control", "public, max-age=" + std::to_string(max_age_seconds));
    }
    void SetPrivateCache(int max_age_seconds = 0)
    {
        AppendHeader("Cache-Control", "private, max-age=" + std::to_string(max_age_seconds));
    }
    void TurnOffCache()
    {
        AppendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    }

    void MarkKeepAlive();
    void MarkClose();

    bool IsKeepAlive() const;
    bool IsClose() const;

    void RemoveLocation()
    {
        _headers.erase("Location");
    }

    bool IsChunked() const;
    void MarkChunked()
    {
        AppendHeader("Transfer-Encoding", "chunked");

        if (_version_major < 1 || _version_minor < 1)
        {
            _version_major =  1;
            _version_minor = 1;
        }
    }

    void SetCookie(std::string const& name, std::string const& value)
    {
        Cookie cookie(name, value);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetSecureCookie(std::string const& name, std::string const& value)
    {
        Cookie cookie(name, value);
        cookie.SetSecure(Cookie::Secure::for_https_only);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetCookie(std::string const& name, std::string const& value, Cookie::HttpOnly http_only)
    {
        Cookie cookie(name, value);
        cookie.SetHttpOnly(http_only);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetSecureCookie(std::string const& name, std::string const& value, Cookie::HttpOnly http_only)
    {
        Cookie cookie(name, value);
        cookie.SetSecure(Cookie::Secure::for_https_only);
        cookie.SetHttpOnly(http_only);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetCookie(std::string const& name, std::string const& value
                   , std::size_t max_age, Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_and_js)
    {
        Cookie cookie(name, value);
        cookie.SetMaxAge(max_age);
        cookie.SetHttpOnly(http_only);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetSecureCookie(std::string const& name, std::string const& value
                         , std::size_t max_age, Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_and_js)
    {
        Cookie cookie(name, value);
        cookie.SetSecure(Cookie::Secure::for_https_only);
        cookie.SetMaxAge(max_age);
        cookie.SetHttpOnly(http_only);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetCookie(std::string const& name, std::string const& value
                   , std::string const& domain, std::string const& path
                   , std::size_t max_age, Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_and_js)
    {
        Cookie cookie(name, value, domain, path);
        cookie.SetHttpOnly(http_only);
        cookie.SetMaxAge(max_age);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetSecureCookie(std::string const& name, std::string const& value
                         , std::string const& domain, std::string const& path
                         , std::size_t max_age, Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_and_js)
    {
        Cookie cookie(name, value, domain, path);
        cookie.SetSecure(Cookie::Secure::for_https_only);
        cookie.SetHttpOnly(http_only);
        cookie.SetMaxAge(max_age);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetCookie(Cookie const& cookie);

    void SetCookieExpiredImmediately(std::string const& name
                                     , std::string const& domain = ""
                                     , std::string const& path = "");

    std::vector<Cookie> const& GetCookies() const
    {
        return _cookies;
    }

    std::vector<Cookie>& GetCookies()
    {
        return _cookies;
    }

    void Reset();

public:
    void Status(int code, std::string body = EmptyBody());

    void Ok(std::string body = EmptyBody());
    void Continue()
    {
        _status_code = HTTP_STATUS_CONTINUE;
    }

    void Nofound(std::string body = EmptyBody());
    void Gone(std::string body = EmptyBody());

    void Unauthorized(std::string body = EmptyBody());
    void NoAuthoritativeInformation(std::string body = EmptyBody());
    void BadRequest(std::string body = EmptyBody());
    void RangeNotSatisfiable(std::string body = EmptyBody());
    void Forbidden(std::string body = EmptyBody());
    void MethodNotAllowed(std::string body = EmptyBody());
    void HttpVersionNotSupported(std::string body = EmptyBody());

    void PayloadTooLarge(std::string body = EmptyBody());
    void UriTooLong(std::string body = EmptyBody());
    void TooManyRequests(std::string body = EmptyBody());

    void LengthRequired()
    {
        _status_code = HTTP_STATUS_LENGTH_REQUIRED;
    }

    void NotImplemented(std::string body = EmptyBody());
    void UnsupportedMediaType(std::string body = EmptyBody());

    void ServiceUnavailable(std::string body = EmptyBody());
    void InternalServerError(std::string body = EmptyBody());

    void MovedPermanently(std::string const& dst_location, std::string body = EmptyBody());

    enum class RedirectType {temporary,  permanent};
    void Redirect(std::string const& dst_location
                  , RedirectType type = RedirectType::temporary
                  , std::string body = EmptyBody());

private:
    void set_or_default_body(std::string body, bool provide_default_if_body_is_empty = true);

private:
    int _status_code = HTTP_STATUS_OK;

    ICHeaders _headers;
    std::string _body;
    std::string _charset = "UTF-8";

    unsigned short _version_major = 1;
    unsigned short _version_minor = 1;

    std::vector<Cookie> _cookies;
private:
    ChunkedBodies _chunked_bodies;
};

std::ostream& operator << (std::ostream& os, Response const& res);

} //namespace da4qi4

#endif // DAQI_RESPONSE_HPP
