#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <functional>

#include "http-parser/http_parser.h"

#include "def/def.hpp"
#include "def/boost_def.hpp"

namespace da4qi4
{

std::string const&  EmptyBody();

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

    enum ContentTypeValuePart {content_type_without_chartset = 0, content_with_chartset = 1};
    std::string GetContentType(ContentTypeValuePart part = content_with_chartset) const;
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

public:
    void Status(int code, std::string const& body = EmptyBody());

    void Ok(std::string const& body = EmptyBody());
    void Continue()
    {
        _status_code = HTTP_STATUS_CONTINUE;
    }

    void Nofound(std::string const& body = EmptyBody());
    void Gone(std::string const& body = EmptyBody());

    void Unauthorized(std::string const& body = EmptyBody());
    void NoAuthoritativeInformation(std::string const& body = EmptyBody());
    void BadRequest(std::string const& body = EmptyBody());
    void RangeNotSatisfiable(std::string const& body = EmptyBody());
    void Forbidden(std::string const& body = EmptyBody());
    void MethodNotAllowed(std::string const& body = EmptyBody());
    void HttpVersionNotSupported(std::string const& body = EmptyBody());

    void PayloadTooLarge(std::string const& body = EmptyBody());
    void UriTooLong(std::string const& body = EmptyBody());
    void TooManyRequests(std::string const& body = EmptyBody());
    void LengthRequired()
    {
        _status_code = HTTP_STATUS_LENGTH_REQUIRED;
    }

    void NotImplemented(std::string const& body = EmptyBody());
    void UnsupportedMediaType(std::string const& body = EmptyBody());

    void ServiceUnavailable(std::string const& body = EmptyBody());
    void InternalServerError(std::string const& body = EmptyBody());

    void MovedPermanently(std::string const& dst_location, std::string const& body = EmptyBody());

    enum class RedirectType {temporary,  permanent};
    void Redirect(std::string const& dst_location
                  , RedirectType type = RedirectType::temporary
                  , std::string const& body = EmptyBody());

private:
    void set_or_default_body(std::string const& body, bool provide_default_if_body_is_empty = true);

private:
    int _status_code = HTTP_STATUS_OK;

    ICHeaders _headers;
    std::string _body;
    std::string _charset = "utf-8";

    unsigned short _version_major = 1;
    unsigned short _version_minor = 1;

};

std::ostream& operator << (std::ostream& os, Response const& res);

} //namespace da4qi4

#endif // RESPONSE_HPP
