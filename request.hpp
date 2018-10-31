#ifndef DAQI_REQUEST_HPP
#define DAQI_REQUEST_HPP

#include <string>
#include <map>
#include <bitset>
#include <vector>

#include "http-parser/http_parser.h"

#include "def/def.hpp"
#include "def/boost_def.hpp"
#include "utilities/container_utilities.hpp"

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
    struct SubHeaders
    {
        std::string value;
        ICHeaders headers;

        bool IsEmpty() const
        {
            return value.empty();
        }
    };

    MultiPart() = default;
    ~MultiPart() = default;

    MultiPart(MultiPart const& o);
    MultiPart(MultiPart&& o);

    size_t HeaderCount() const
    {
        return _headers.size();
    }
    size_t DataSize() const
    {
        return _data.size();
    }

    bool IsExistsHeader(std::string const& field) const;
    std::string const& GetHeader(std::string const& field) const;
    OptionalStringRefConst TryGetHeader(std::string const& field) const;

    void AppendHeader(std::string&& field, std::string&& value);

    ICHeaders& GetHeaders()
    {
        return _headers;
    }
    ICHeaders const& GetHeaders() const
    {
        return _headers;
    }

    std::string const& GetData() const
    {
        return _data;
    }

    std::string&& GetData()
    {
        return std::move(_data);
    }

    void SetData(std::string&& data)
    {
        _data = std::move(data);
    }

    SubHeaders GetSubHeaders(std::string const& field);

    void ClearData()
    {
        _data.clear();
        _data.shrink_to_fit();
    }

    void Clear()
    {
        _headers.clear();
        _data.clear();
    }
private:
    ICHeaders _headers;
    std::string _data;
};

struct FormDataItem
{
    std::string name;
    enum DataFlag {is_data, is_file_data, is_file_temporary_name};
    DataFlag data_flag;
    std::string filename;
    std::string content_type;
    std::string data;

    bool IsData() const
    {
        return data_flag == is_data || data_flag == is_file_data;
    }

    bool IsFile() const
    {
        return data_flag == is_file_data || data_flag == is_file_temporary_name;
    }

    bool IsSavedFile() const
    {
        return data_flag == is_file_temporary_name;
    }

    std::string const& GetSavedFileTemporaryName() const
    {
        return (!IsSavedFile() ? Utilities::theEmptyString : data);
    }

    void Reset()
    {
        name.clear();
        data_flag = is_data;
        filename.clear();
        content_type.clear();
        data.clear();
    }
};

class Request
{
public:
    enum ParameterSrc
    {
        fromUnknown = 0, fromUrl, fromForm, fromHeader, fromCookie
    };

    bool IsExistsHeader(std::string const& field) const;
    std::string const& GetHeader(std::string const& field) const;
    OptionalStringRefConst TryGetHeader(std::string const& field) const;

    bool IsExistsUrlParameter(std::string const& name) const;
    std::string const& GetUrlParameter(std::string const& name) const;
    OptionalStringRefConst TryGetUrlParameter(std::string const& name) const;

    bool IsExistsFormData(std::string const& name) const;
    std::string const& GetFormData(std::string const& name) const;
    OptionalStringRefConst TryGetFormData(std::string const& name) const;

    bool IsExistsCookie(std::string const& name) const;
    std::string const& GetCookie(std::string const& name) const;
    OptionalStringRefConst TryGetCookie(std::string const& name) const;

    ParameterSrc IsExistsParameter(std::string const& name) const
    {
        //find order : url -> formdata -> header -> cookie
        return IsExistsHeader(name) ? fromUrl
               : (IsExistsFormData(name) ? fromForm
                  : (IsExistsHeader(name) ?  fromHeader
                     : (IsExistsCookie(name) ? fromCookie
                        : fromUnknown)));
    }

    std::string const& GetParameter(std::string const& name) const;
    OptionalStringRefConst TryGetParameter(std::string const& name) const;
    std::string const& operator[](std::string const& name) const
    {
        return GetParameter(name);
    }

    Url const& GetUrl() const
    {
        return _url;
    }

    http_method GetMethod() const
    {
        return static_cast<http_method>(_method);
    }

    std::string GetMethodName() const
    {
        return http_method_str(GetMethod());
    }

    ICHeaders const& GetHeader() const
    {
        return _headers;
    }

    ICHeaders const& GetCookies() const
    {
        return _cookies;
    }

    std::string const& GetMultiPartBoundary() const
    {
        return _boundary;
    }

    std::vector<MultiPart>& GetMultiParts()
    {
        return _multiparts;
    }

    std::vector<MultiPart> const& GetMultiParts() const
    {
        return _multiparts;
    }

    void GetVersion(unsigned short& major, unsigned short& minor) const
    {
        major = _version_major;
        minor = _version_minor;
    }

    std::pair<unsigned short, unsigned short> GetVersion()
    {
        return {_version_major, _version_minor};
    }

    bool IsContentLengthProvided() const
    {
        return (_flags & F_CONTENTLENGTH) != 0;
    }
    uint64_t GetContentLength() const
    {
        return _content_length;
    }

    bool IsChunked() const
    {
        return (_flags & F_CHUNKED) != 0;
    }
    bool IsUpgrade() const
    {
        return _addition_flags[upgrade_bit];
    }
    bool IsFormData() const
    {
        return _addition_flags[formdata_bit];
    }
    bool IsMultiPart() const
    {
        return _addition_flags[multipart_bit];
    }
    bool IsKeepAlive() const
    {
        return _addition_flags[keepalive_bit];
    }
    bool IsBodySkipped() const
    {
        return (_flags & F_SKIPBODY) != 0;
    }

    bool HasBody() const
    {
        return !_body.empty();
    }

public:
    bool ParseUrl(std::string&& url);
    void ParseContentType();

    void AppendHeader(std::string&& field, std::string&& value);

    void MarkUpgrade(bool upgrade)
    {
        _addition_flags.set(upgrade_bit, upgrade);
    }
    void MarkKeepAlive(bool keep)
    {
        _addition_flags.set(keepalive_bit, keep);
    }
    void MarkMultiPart(bool multipart)
    {
        _addition_flags.set(multipart_bit, multipart);
    }
    void MarkFormData(bool formdata)
    {
        _addition_flags.set(formdata_bit, formdata);
    }

    void SetMethod(unsigned int method)
    {
        _method = method;
    }
    void SetVersion(unsigned short major, unsigned short minor)
    {
        _version_major = major;
        _version_minor = minor;
    }
    void SetContentLength(uint64_t content_length)
    {
        _content_length = content_length;
    }
    void SetFlags(unsigned int flags)
    {
        _flags = flags;
    }
    void SetBody(std::string&& body)
    {
        _body.swap(body);
    }
    void SetMultiPartBoundary(char const* at, size_t length);

    void AddMultiPart(MultiPart&& part)
    {
        _multiparts.push_back(std::move(part));
    }

    void TransferHeadersToCookies();
    void TransferMultiPartsToFormData(UploadFileSaveOptions const& options, std::string const& dir);

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

    ICHeaders _cookies;
    std::vector<FormDataItem> _formdata;
};

} //namespace da4qi4

#endif // DAQI_REQUEST_HPP
