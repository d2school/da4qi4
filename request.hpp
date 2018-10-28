#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <bitset>
#include <vector>

#include "http-parser/http_parser.h"

#include "def/def.hpp"
#include "def/boost_def.hpp"

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

struct MultiPart
{
    MultiPart() = default;
    ~MultiPart() = default;
    
    MultiPart(MultiPart const& o);
    MultiPart(MultiPart&& o);
    
    size_t HeaderCount() const { return _headers.size(); }
    size_t DataSize() const  { return _data.size(); }
    
    bool IsExistsHeader(std::string const& field) const;
    std::string const& GetHeaderValue(std::string const& field) const;
    OptionalStringRefConst TryGetHeaderValue(std::string const& field) const;
    
    void AddHeader(std::string&& field, std::string&& value)
    {
        _headers.emplace(std::make_pair(std::move(field), std::move(value)));
    }
    
    ICHeaders& GetHeaders() { return _headers; }
    ICHeaders const& GetHeaders() const { return _headers; }
    
    std::string const& GetData() const { return _data; }
    void SetData(std::string&& data) { _data.swap(data); }
    void ClearData() { _data.clear(); _data.shrink_to_fit();}
    
    void Clear()
    {
        _headers.clear();
        _data.clear();
    }
private:
    ICHeaders _headers;
    std::string _data;
};

class Request
{
public:
    bool IsExistsHeader(std::string const& field) const;
    std::string const& GetHeaderValue(std::string const& field) const;
    OptionalStringRefConst TryGetHeaderValue(std::string const& field) const;
    
    bool IsExistsUrlParameter(std::string const& name) const;
    std::string const& GetUrlParameter(std::string const& name) const;
    OptionalStringRefConst TryGetUrlParameter(std::string const& name) const;
    
    Url const& GetUrl() const  { return _url; }
    ICHeaders const& GetHeader() const { return _headers; }
    http_method GetMethod() const { return static_cast<http_method>(_method); }
    std::string GetMethodName() const { return http_method_str(GetMethod()); }
    
    std::string const& GetMultiPartBoundary() const { return _boundary;  }
    
    void GetVersion(unsigned short& major, unsigned short& minor) const
    {
        major = _version_major;
        minor = _version_minor;
    }
    
    std::pair<unsigned short, unsigned short> GetVersion() { return {_version_major, _version_minor}; }
    
    bool IsContentLengthProvided() const { return (_flags & F_CONTENTLENGTH) != 0; }
    uint64_t GetContentLength() const { return _content_length; }
    
    bool IsChunked() const { return (_flags & F_CHUNKED) != 0; }
    bool IsUpgrade() const { return _addition_flags[upgrade_bit]; }
    bool IsFormData() const { return _addition_flags[formdata_bit]; }
    bool IsMultiPart() const { return _addition_flags[multipart_bit]; }
    bool IsKeepAlive() const { return _addition_flags[keepalive_bit]; }
    bool IsBodySkipped() const { return (_flags & F_SKIPBODY) != 0; }
    
    bool HasBody() const { return !_body.empty(); }
    
public:
    bool ParseUrl(std::string&& url);
    void ParseContentType();
    
    void AppendHeader(std::string&& field, std::string&& value);
    
    void MarkUpgrade(bool upgrade) { _addition_flags.set(upgrade_bit, upgrade); }
    void MarkKeepAlive(bool keep) { _addition_flags.set(keepalive_bit, keep); }
    void MarkMultiPart(bool multipart) { _addition_flags.set(multipart_bit, multipart); }
    void MarkFormData(bool formdata) { _addition_flags.set(formdata_bit, formdata); }
    
    void SetMethod(unsigned int method) { _method = method; }
    void SetVersion(unsigned short major, unsigned short minor) {_version_major = major; _version_minor = minor;}
    void SetContentLength(uint64_t content_length) { _content_length = content_length; }
    void SetFlags(unsigned int flags) { _flags = flags; }
    void SetBody(std::string&& body) { _body.swap(body); }
    void SetMultiPartBoundary(char const* at, size_t length);
    
    void AddMultiPart(MultiPart&& part) { _multiparts.push_back(std::move(part)); }
    
    void Reset();
    
    std::string dump() const;
    
private:
    static int const upgrade_bit = 0;
    static int const keepalive_bit = 1;
    static int const multipart_bit = 2;
    static int const formdata_bit = 3;
    
    std::bitset<4> _addition_flags;
    
    Url _url;
    unsigned int _method = HTTP_GET;
    ICHeaders _headers;
    
    unsigned short _version_major = 0;
    unsigned short _version_minor = 0;
    
    uint64_t _content_length = 0L;
    unsigned int _flags = 0;
    
    std::string _body;
    
    std::string _boundary;
    std::vector<MultiPart> _multiparts;
};

} //namespace da4qi4

#endif // REQUEST_HPP
