#include "request.hpp"

#include <iostream>

namespace da4qi4
{

bool Request::IsExistsHeader(std::string const& field) const
{
    auto const it = _headers.find(field);
    return it != _headers.cend();
}

std::string const& Request::GetHeaderValue(std::string const& field) const
{
    auto const it = _headers.find(field);
    return (it != _headers.cend()? it->second : Utilities::EmptyString());    
}

Utilities::OptionalStringRefConst Request::TryGetHeaderValue(std::string const& field) const
{
    auto const it = _headers.find(field);
    return (it == _headers.cend() ? Utilities::OptionalStringRefConst(boost::none) 
                                  : Utilities::OptionalStringRefConst(it->second));
}

bool Request::IsExistsUrlParameter(std::string const& name) const
{
    auto const it = _url.paramters.find(name);
    return it != _url.paramters.cend();
}

std::string const& Request::GetUrlParameter(std::string const& name) const
{
    auto const it = _url.paramters.find(name);
    return (it != _url.paramters.cend()? it->second : Utilities::EmptyString());    
}

Utilities::OptionalStringRefConst Request::TryGetUrlParameter(std::string const& name) const
{
    auto const it = _url.paramters.find(name);
    return (it == _url.paramters.cend() ? Utilities::OptionalStringRefConst(boost::none)
                                        : Utilities::OptionalStringRefConst(it->second)); 
}

bool GetURLPartValue(int url_part_flag,  Url& url, std::string&& value)
{
    switch (url_part_flag)
    {
    case UF_SCHEMA :
        url.schema.swap(value);
        break;
    case UF_HOST :
        url.host.swap(value);
        break;
    case UF_PORT:
        break; //skip, but return true;
    case UF_PATH :
        url.path.swap(value);
        break;
    case UF_QUERY :
        url.query.swap(value);
        break;
    case UF_FRAGMENT :
        url.fragment.swap(value);
        break;
    case UF_USERINFO :
        url.userinfo.swap(value);
        break;
    default:
        return false;
    }

    return true;
}

bool Request::ParseUrl(std::string&& url)
{
    http_parser_url r;
    http_parser_url_init(&r);

    int err = http_parser_parse_url(url.c_str(), url.length(), 0, &r);

    if (0 == err)
    {
        _url.port = r.port;

        for (unsigned int i=0; i < UF_MAX; ++i)
        {
            if ((r.field_set & (1 << i)) == 0)
            {
                continue;
            }

            GetURLPartValue(i, _url, std::string(url.c_str() + r.field_data[i].off, r.field_data[i].len));
        }
    }

    _url.value.swap(url);

    return !err;
}


void Request::AppendHeader(std::string&& field, std::string&& value)
{
    _headers.emplace(std::make_pair(std::move(field), std::move(value)));
}

void Request::Reset()
{
    _url.Clear();
    _method = HTTP_GET;
    _headers.clear();

    _version_major = _version_minor = 0;

    _content_length = 0;
    _flags = 0;
    _addition_flags.reset();    

    if (_body.length() < 1024 * 10)
    {
        _body.clear();
    }
    else
    {
        std::string tmp;
        _body.swap(tmp);
        _body.reserve(1024 * 2);
    }

    _boundary.clear();
}

void Request::ParseContentType()
{
    auto content_type = this->TryGetHeaderValue("Content-Type");
    
    if (content_type)
    {
        auto beg = content_type->find("multipart/");
        if (beg != std::string::npos)
        {
            constexpr int len_of_multipart_flag = 10; // length of "multipart/"
            constexpr int len_of_boundary_flag = 9;   // length of "boundary="
            
            this->MarkMultiPart(true);
            if (content_type->find("form-data", beg + len_of_multipart_flag) != std::string::npos)
            {
                this->MarkFormData(true);
            }
            
            auto pos = content_type->find("boundary=", beg + len_of_multipart_flag);
            
            if (pos != std::string::npos)
            {
                _boundary = content_type->substr(pos + len_of_boundary_flag);
            }
        }
    }
}

void Request::SetMultiPartBoundary(char const* at, size_t length)
{
    _boundary = std::string(at, length);
}


void Request::Dump() const
{
    std::cout << "====== URL ======\r\n";
    std::cout << "url : " << _url.value << std::endl;
    std::cout << "host : " <<_url.host << std::endl;
    std::cout << "port : " <<_url.port << std::endl;
    std::cout << "path : " <<_url.path << std::endl;
    std::cout << "query : " <<_url.query << std::endl;
    std::cout << "fragment : " <<_url.fragment << std::endl;
    std::cout << "userinfo : " <<_url.userinfo << std::endl;

    std::cout << "====== URL PARAMETERS ======\r\n";
    for (auto const& p : _url.paramters)
    {
        std::cout << p.first << " : " << p.second << std::endl;
    }

    std::cout << "====== METHOD ======\r\n";
    std::cout << "method : " << http_method_str(static_cast<http_method>(_method)) << std::endl;

    std::cout << "====== HEADERS ======\r\n";
    for (auto const& p :_headers)
    {
        std::cout << p.first << " : " << p.second << "\r\n";
    }

    std::cout << "====== FLAGS ======\r\n";
    std::cout << "upgrade : " << std::boolalpha << this->IsUpgrade() << "\r\n";
    std::cout << "has content-length : " << std::boolalpha << this->IsContentLengthProvided() << "\r\n";
    std::cout << "chunked : " << std::boolalpha << this->IsChunked() << "\r\n";
    std::cout << "multipart : " << std::boolalpha << this->IsMultiPart() << "\r\n";
    std::cout << "formdata : " << std::boolalpha << this->IsFormData() << "\r\n";
    std::cout << "keepalive : " << std::boolalpha << this->IsKeepAlive() << "\r\n";
    
    if (this->IsMultiPart()) 
    {
        std::cout << "boundary : " << GetMultiPartBoundary() << "\r\n";
    }

    std::cout << "====== BODY ======\r\n";
    std::cout << _body << std::endl;
}

}//namespace da4qi4
