#ifndef DAQI_RESPONSE_HPP
#define DAQI_RESPONSE_HPP

#include <iostream>
#include <functional>
#include <vector>

#include <mutex>

#include "llhttp/llhttp.h"
#include "llhttp/helper/http_status_def.h"

#include "daqi/def/def.hpp"
#include "daqi/def/boost_def.hpp"
#include "daqi/def/json_def.hpp"

#include "daqi/cookie.hpp"

namespace da4qi4
{

std::string const&  EmptyBody();

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

    void CacheControlMaxAge(int max_age_seconds = 0, CacheControl cc = CacheControl::Public)
    {
        std::string s = (cc == CacheControl::Public) ? "public" : "private";
        AppendHeader("Cache-Control", s + ", max-age=" + std::to_string(max_age_seconds));
    }

    void NoCache()
    {
        AppendHeader("Cache-Control", "no-cache");
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
        cookie.SetMaxAge(static_cast<int>(max_age));
        cookie.SetHttpOnly(http_only);
        cookie.ApplyHttpVersion(_version_major, _version_minor);
        SetCookie(cookie);
    }

    void SetSecureCookie(std::string const& name, std::string const& value
                         , std::size_t max_age, Cookie::HttpOnly http_only = Cookie::HttpOnly::for_http_and_js)
    {
        Cookie cookie(name, value);
        cookie.SetSecure(Cookie::Secure::for_https_only);
        cookie.SetMaxAge(static_cast<int>(max_age));
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
        cookie.SetMaxAge(static_cast<int>(max_age));
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
        cookie.SetMaxAge(static_cast<int>(max_age));
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
    void ReplyStatus(int code, std::string body = EmptyBody());

    void ReplyOk();
    void ReplyOk(const char* body);
    void ReplyOk(std::string body);
    void ReplyOk(Json const& result);

    void ReplyOkJSON(std::string json_string,
                     std::string const& content_type = "application/json",
                     std::string const& content_encoding = "utf-8")
    {
        this->SetContentEncoding(content_encoding);
        this->SetContentType(content_type);
        ReplyOk(json_string);
    }

    void ReplyContinue()
    {
        _status_code = HTTP_STATUS_CONTINUE;
    }

    void ReplyNofound(std::string body = EmptyBody());
    void ReplyGone(std::string body = EmptyBody());

    void ReplyUnauthorized(std::string body = EmptyBody());
    void ReplyNoAuthoritativeInfo(std::string body = EmptyBody());
    void ReplyBadRequest(std::string body = EmptyBody());
    void ReplyRangeNotSatisfiable(std::string body = EmptyBody());
    void ReplyForbidden(std::string body = EmptyBody());
    void ReplyMethodNotAllowed(std::string body = EmptyBody());
    void ReplyHttpVersionNotSupported(std::string body = EmptyBody());

    void ReplyPayloadTooLarge(std::string body = EmptyBody());
    void ReplyUriTooLong(std::string body = EmptyBody());
    void ReplyTooManyRequests(std::string body = EmptyBody());
    void ReplyLengthRequired()
    {
        _status_code = HTTP_STATUS_LENGTH_REQUIRED;
    }
    void ReplyNotImplemented(std::string body = EmptyBody());
    void ReplyUnsupportedMediaType(std::string body = EmptyBody());

    void ReplyServiceUnavailable(std::string body = EmptyBody());
    void ReplyInternalServerError(std::string body = EmptyBody());

    void ReplyMovedPermanently(std::string const& dst_location, std::string body = EmptyBody());

    enum class RedirectType {temporary,  permanent};
    void ReplyRedirect(std::string const& dst_location
                       , RedirectType type = RedirectType::temporary
                       , std::string body = EmptyBody());

private:
    void set_or_default_body(std::string body, bool provide_default_if_body_is_empty = true);
    void add_server_header();
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
