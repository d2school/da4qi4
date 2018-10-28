#include "request.hpp"

#include "def/debug_def.hpp"

#include <sstream>

namespace da4qi4
{

MultiPart::MultiPart(MultiPart const& o)
    : _headers(o._headers), _data(o._data)
{
}

MultiPart::MultiPart(MultiPart&& o)
    : _headers(std::move(o._headers)), _data(std::move(o._data))
{
}

bool MultiPart::IsExistsHeader(std::string const& field) const
{
    auto const it = _headers.find(field);
    return it != _headers.cend();
}

std::string const& MultiPart::GetHeaderValue(std::string const& field) const
{
    auto const it = _headers.find(field);
    return (it != _headers.cend() ? it->second : Utilities::theEmptyString);
}

OptionalStringRefConst MultiPart::TryGetHeaderValue(std::string const& field) const
{
    auto const it = _headers.find(field);
    return (it == _headers.cend() ? OptionalStringRefConst(NoneObject)
            : OptionalStringRefConst(it->second));
}

bool Request::IsExistsHeader(std::string const& field) const
{
    auto const it = _headers.find(field);
    return it != _headers.cend();
}

std::string const& Request::GetHeaderValue(std::string const& field) const
{
    auto const it = _headers.find(field);
    return (it != _headers.cend() ? it->second : Utilities::theEmptyString);
}

OptionalStringRefConst Request::TryGetHeaderValue(std::string const& field) const
{
    auto const it = _headers.find(field);
    return (it == _headers.cend() ? OptionalStringRefConst(NoneObject)
            : OptionalStringRefConst(it->second));
}

bool Request::IsExistsUrlParameter(std::string const& name) const
{
    auto const it = _url.parameters.find(name);
    return it != _url.parameters.cend();
}

std::string const& Request::GetUrlParameter(std::string const& name) const
{
    auto const it = _url.parameters.find(name);
    return (it != _url.parameters.cend() ? it->second : Utilities::theEmptyString);
}

OptionalStringRefConst Request::TryGetUrlParameter(std::string const& name) const
{
    auto const it = _url.parameters.find(name);
    return (it == _url.parameters.cend() ? OptionalStringRefConst(NoneObject)
            : OptionalStringRefConst(it->second));
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
        
        for (unsigned int i = 0; i < UF_MAX; ++i)
        {
            if ((r.field_set & (1 << i)) == 0)
            {
                continue;
            }
            
            GetURLPartValue(i, _url, std::string(url.c_str() + r.field_data[i].off, r.field_data[i].len));
        }
    }
    
    _url.full.swap(url);
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
    _multiparts.clear();
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


std::string Request::dump() const
{
    std::stringstream ss;
    
    ss << "====== URL ======\r\n";
    ss << "url : " << _url.full <<  "\r\n";
    ss << "host : " << _url.host <<  "\r\n";
    ss << "port : " << _url.port <<  "\r\n";
    ss << "path : " << _url.path <<  "\r\n";
    ss << "query : " << _url.query <<  "\r\n";
    ss << "fragment : " << _url.fragment <<  "\r\n";
    ss << "userinfo : " << _url.userinfo <<  "\r\n";
    ss << "====== URL PARAMETERS ======\r\n";
    
    for (auto const& p : _url.parameters)
    {
        ss << p.first << " : " << p.second <<  "\r\n";
    }
    
    ss << "====== METHOD ======\r\n";
    ss << "method : " << this->GetMethodName() <<  "\r\n";
    ss << "====== HEADERS ======\r\n";
    
    for (auto const& p : _headers)
    {
        ss << p.first << " : " << p.second << "\r\n";
    }
    
    ss << "====== FLAGS ======\r\n";
    ss << "upgrade : " << std::boolalpha << this->IsUpgrade() << "\r\n";
    ss << "has content-length : " << std::boolalpha << this->IsContentLengthProvided() << "\r\n";
    ss << "chunked : " << std::boolalpha << this->IsChunked() << "\r\n";
    ss << "multipart : " << std::boolalpha << this->IsMultiPart() << "\r\n";
    ss << "formdata : " << std::boolalpha << this->IsFormData() << "\r\n";
    ss << "keepalive : " << std::boolalpha << this->IsKeepAlive() << "\r\n";
    
    if (this->IsMultiPart())
    {
        ss << "boundary : " << GetMultiPartBoundary() << "\r\n";
    }
    
    ss << "====== BODY ======\r\n";
    ss << _body <<  "\r\n";
    ss << "======MULTIPART======\r\n";
    
    for (auto const& mp : this->_multiparts)
    {
        for (auto const& h : mp.GetHeaders())
        {
            ss << h.first << " = " << h.second << "\r\n";
        }
        
        ss << "part data : \r\n";
        ss << mp.GetData() <<  "\r\n";
    }
    
    return ss.str();
}

}//namespace da4qi4
