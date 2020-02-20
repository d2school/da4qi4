#include "daqi/request.hpp"

#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "daqi/def/log_def.hpp"

#include "daqi/utilities/string_utilities.hpp"
#include "daqi/utilities/file_utilities.hpp"
#include "daqi/utilities/html_utilities.hpp"
#include "daqi/utilities/http_utilities.hpp"

#include "daqi/application.hpp"

namespace da4qi4
{

void RoutingPathParameters::InitParameters(std::vector<std::string> const& names
                                           , std::vector<std::string> const& values)
{
    _parameters.clear();

    for (size_t i = 0; i < names.size(); ++i)
    {
        std::string value = (i < values.size()) ? values[i] : Utilities::theEmptyString;
        _parameters.insert(std::make_pair(names[i], value));
    }
}

bool RoutingPathParameters::IsExists(std::string const& name) const
{
    return _parameters.find(name) != _parameters.cend();
}

std::string const& RoutingPathParameters::Get(std::string const& name) const
{
    auto it = _parameters.find(name);
    return (it != _parameters.cend() ? it->second : Utilities::theEmptyString);
}

OptionalStringRefConst RoutingPathParameters::TryGet(std::string const& name) const
{
    auto it = _parameters.find(name);
    return (it != _parameters.cend() ? OptionalStringRefConst(it->second)
            : OptionalStringRefConst(NoneObject));
}

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
    return Utilities::IsExistsHeader(_headers, field);
}

std::string const& MultiPart::GetHeader(std::string const& field) const
{
    return Utilities::GetHeader(_headers, field);
}

OptionalStringRefConst MultiPart::TryGetHeader(std::string const& field) const
{
    return Utilities::TryGetHeader(_headers, field);
}

void MultiPart::AppendHeader(std::string&& field, std::string&& value)
{
    auto it = _headers.find(field);

    if (it != _headers.end())
    {
        it->second = std::move(value);
    }
    else
    {
        _headers.insert(std::make_pair(std::move(field), std::move(value)));
    }
}

bool Request::IsExistsHeader(std::string const& field) const
{
    return Utilities::IsExistsHeader(_headers, field);
}

std::string const& Request::GetHeader(std::string const& field) const
{
    return Utilities::GetHeader(_headers, field);
}

OptionalStringRefConst Request::TryGetHeader(std::string const& field) const
{
    return Utilities::TryGetHeader(_headers, field);
}

bool Request::IsExistsUrlParameter(std::string const& name) const
{
    return Utilities::IsExistsHeader(_url.parameters, name);
}

std::string const& Request::GetUrlParameter(std::string const& name) const
{
    return Utilities::GetHeader(_url.parameters, name);
}

OptionalStringRefConst Request::TryGetUrlParameter(std::string const& name) const
{
    return Utilities::TryGetHeader(_url.parameters, name);
}

bool UploadFile::SaveTo(std::string const& dst_filename)
{
    if (_status == no_found)
    {
        return false;
    }

    std::ofstream ofs(dst_filename);

    if (!ofs)
    {
        return false;
    }

    if (_status == in_stream)
    {
        if (!_stream)
        {
            return false;
        }

        size_t const tmp_buf_size = 2048;
        char buf[tmp_buf_size];

        while (_stream)
        {
            _stream.read(buf, tmp_buf_size);
            std::size_t rd = static_cast<std::size_t>(_stream.gcount());

            if (rd > 0)
            {
                ofs.write(buf, static_cast<std::streamsize>(rd));
            }
        }

        _stream.close();
    }
    else // in_memory
    {
        ofs.write(_memory.c_str(), static_cast<std::streamsize>(_memory.size()));
        _memory.clear();
    }

    ofs.close();
    return true;
}

bool UploadFile::ToMemory()
{
    if ((_status == in_stream && !_stream) || (_status == no_found))
    {
        return false;
    }

    if (_status == in_memory)
    {
        return true;
    }

    size_t const tmp_buf_size = 2048;
    char buf[tmp_buf_size];
    _memory.clear();
    _memory.reserve(tmp_buf_size << 1);

    while (_stream)
    {
        _stream.read(buf, tmp_buf_size);
        std::size_t rd = static_cast<std::size_t>(_stream.gcount());

        if (rd > 0)
        {
            _memory.append(buf, rd);
        }
    }

    _stream.close();
    _status = in_memory;

    return true;
}

bool UploadFile::ToStream(std::string const& temp_filename)
{
    if (_status == no_found)
    {
        return false;
    }

    if (_status == in_stream)
    {
        return true;
    }

    std::string err;

    if (!Utilities::SaveDataToFile(this->_memory, temp_filename, err))
    {
        log::Server()->error("Try create temp file for upload fail. {}. {}. {}."
                             , temp_filename, _src_filename, err);
        return false;
    }

    std::ofstream ofs(temp_filename.c_str(), std::ios_base::out | std::ios_base::binary);

    if (_stream.is_open())
    {
        _stream.close();
    }

    _stream.open(temp_filename.c_str(), std::ios_base::in | std::ios_base::binary);

    if (!_stream)
    {
        log::Server()->error("Try open temp file for upload fail. {}. {}."
                             , temp_filename, _src_filename);
        return false;
    }

    _saved_filename = temp_filename;
    return  true;
}

extern fs::path MakeUploadFileTemporaryName(std::string const& ext, std::string const& dir);

bool UploadFile::ToStream(const Application& app, std::string const& ext)
{
    if (_status == no_found)
    {
        return false;
    }

    if (_status == in_stream)
    {
        return true;
    }

    auto upload_root = app.GetUploadRoot().string();

    if (upload_root.empty())
    {
        errorcode ec;
        upload_root = fs::temp_directory_path(ec).native();

        if (ec)
        {
            log::Server()->error("Get system temp file path fail. {}.", ec.message());
            return false;
        }
    }

    auto templ_file_path = MakeUploadFileTemporaryName(ext, upload_root);

    if (templ_file_path.empty())
    {
        return false;
    }

    return ToStream(templ_file_path.native());
}

bool Request::IsExistsFile(std::string const& field_name) const
{
    for (auto const& item : _formdata)
    {
        if (item.IsFile() && Utilities::iEquals(item.name, field_name))
        {
            return true;
        }
    }

    return false;
}

bool UploadFile::FromFormDataItem(FormDataItem const& item)
{
    _field_name = item.name;
    _content_type = item.content_type;

    if (item.IsSavedFile())
    {
        _status = UploadFile::in_stream;
        _stream.open(item.GetSavedFileTemporaryName(), std::ios_base::in | std::ios_base::binary);
        _src_filename = item.filename;
        _saved_filename = item.GetSavedFileTemporaryName();
        return true;
    }

    if (item.IsFile()) //but no saved still
    {
        _status = UploadFile::in_memory;
        _memory = item.data;
        _src_filename = item.filename;
        return true;
    }

    _status = no_found;
    return false;
}

UploadFile Request::GetFile(std::string const& field_name) const
{
    UploadFile uf;

    for (auto const& item : _formdata)
    {
        if (Utilities::iEquals(item.name, field_name) && uf.FromFormDataItem(item))
        {
            return uf;
        }
    }

    return uf;
}

bool Request::IsExistsCookie(std::string const& name) const
{
    return Utilities::IsExistsHeader(_cookies, name);
}

std::string const& Request::GetCookie(std::string const& name) const
{
    return Utilities::GetHeader(_cookies, name);
}

OptionalStringRefConst Request::TryGetCookie(std::string const& name) const
{
    return Utilities::TryGetHeader(_cookies, name);
}

bool Request::IsExistsFormData(std::string const& name) const
{
    for (auto const& fd : _formdata)
    {
        if (Utilities::iEquals(fd.name, name))
        {
            return true;
        }
    }

    return false;
}

std::string const& Request::GetFormData(std::string const& name) const
{
    for (auto const& fd : _formdata)
    {
        if (Utilities::iEquals(fd.name, name))
        {
            return fd.data;
        }
    }

    return Utilities::theEmptyString;
}

OptionalStringRefConst Request::TryGetFormData(std::string const& name) const
{
    for (auto const& fd : _formdata)
    {
        if (Utilities::iEquals(fd.name, name))
        {
            return OptionalStringRefConst(fd.data);
        }
    }

    return OptionalStringRefConst(NoneObject);
}

std::string const& Request::GetParameter(std::string const& name) const
{
    ParameterSrc src = IsExistsParameter(name);

    switch (src)
    {
        case fromUrl:
            return GetUrlParameter(name);

        case fromPath:
            return GetPathParameter(name);

        case fromForm:
            return GetFormData(name);

        case fromHeader:
            return GetHeader(name);

        case fromCookie:
            return GetCookie(name);

        case fromUnknown:
            break;
    }

    return Utilities::theEmptyString;
}

OptionalStringRefConst Request::TryGetParameter(std::string const& name) const
{
    ParameterSrc src = IsExistsParameter(name);

    switch (src)
    {
        case fromUrl:
            return TryGetUrlParameter(name);

        case fromPath:
            return TryGetPathParameter(name);

        case fromForm:
            return TryGetFormData(name);

        case fromHeader:
            return TryGetHeader(name);

        case fromCookie:
            return TryGetCookie(name);

        case fromUnknown:
            break;
    }

    return OptionalStringRefConst(NoneObject);
}


bool Request::ParseUrl(std::string&& url)
{
    return _url.Parse(std::move(url));
}

void Request::AppendHeader(std::string&& field, std::string&& value)
{
    auto it = _headers.find(field);

    if (it != _headers.end())
    {
        it->second = std::move(value);
    }
    else
    {
        _headers.insert(std::make_pair(std::move(field), std::move(value)));
    }
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
    _cookies.clear();
    _formdata.clear();
    _path_parameters.Clear();
}

void Request::ParseContentType()
{
    auto content_type = this->TryGetHeader("Content-Type");

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
        else if (content_type->find("application/x-www-form-urlencoded") != std::string::npos)
        {
            this->MarkFormUrlEncoded(true);
        }
        else if (content_type->find("application/octet-stream") != std::string::npos)
        {
            this->MarkOctetStream(true);
        }
    }
}

void Request::ApplyApplication(std::string const& app_url_root)
{
    _url.UnderApplication(app_url_root);
}

void Request::SetMultiPartBoundary(char const* at, size_t length)
{
    _boundary = std::string(at, length);
}

void MultiPartSplitSubHeadersFromValue(std::string const& value, MultiPart::SubHeaders& sub_headers)
{
    std::vector<std::string> parts = Utilities::Split(value, ';');
    sub_headers.value.clear();

    for (auto p : parts)
    {
        auto pos = p.find('=');

        if (pos == std::string::npos)
        {
            if (sub_headers.value.empty())
            {
                sub_headers.value = p;
            }
        }
        else
        {
            std::string f = p.substr(0, pos);
            Utilities::Trim(f);
            std::string v = p.substr(pos + 1);
            Utilities::Trim(v);
            size_t vl = v.size();

            if (vl >= 2)
            {
                if (v[0] == '\"' && v[vl - 1] == '\"')
                {
                    v = v.substr(1, vl - 2);
                }
            }

            if (!f.empty())
            {
                sub_headers.headers.insert(std::make_pair(std::move(f), std::move(v)));
            }
        }
    }
}

MultiPart::SubHeaders MultiPart::GetSubHeaders(std::string const& field)
{
    SubHeaders h;
    std::string const& value = this->GetHeader(field);

    if (!value.empty())
    {
        MultiPartSplitSubHeadersFromValue(value, h);
    }

    return h;
}

void Request::TransferHeadersToCookies()
{
    auto value = GetHeader("Cookie");

    if (!value.empty())
    {
        std::vector<std::string> parts = Utilities::Split(value, ';',
                                                          Utilities::TrimOptions::keep_space);

        for (auto part : parts)
        {
            std::vector<std::string> kv = Utilities::Split(part, '=',
                                                           Utilities::TrimOptions::trim_all);

            if (!kv.empty())
            {
                std::string const& k = kv[0];
                std::string const& v = (kv.size() > 1) ? kv[1] : Utilities::theEmptyString;
                _cookies.insert(std::make_pair(std::move(k), std::move(v)));
            }
        }
    }
}

fs::path MakeUploadFileTemporaryName(std::string const& ext, std::string const& dir)
{
    fs::path disk_fn(dir);

    do
    {
        std::stringstream ss;
        ss << Utilities::GetUUID() << ext;

        disk_fn /= ss.str();
        size_t retry_count = 0;

        try
        {
            if (fs::exists(disk_fn))
            {
                if (++retry_count > 9)
                {
                    log::Server()->error("Too many files in upload directory: {}.", dir);
                    return Utilities::theEmptyString;
                }

                continue;
            }
        }
        catch (std::exception const& e)
        {
            log::Server()->error("Test file {} exists exception. {}", disk_fn.string(), e.what());
            return Utilities::theEmptyString;
        }
    }
    while (false);

    return disk_fn;
}

void Request::ParseFormUrlEncodedData()
{
    if (!this->_body.empty())
    {
        auto parameters = Utilities::ParseQueryParameters(_body);

        for (auto& p : parameters)
        {
            FormDataItem fdi(p.first, std::move(p.second));
            _formdata.push_back(std::move(fdi));
        }
    }
}

bool TransferMultiPartToFormDataItem(MultiPart& mp, FormDataItem& fdi)
{
    MultiPart::SubHeaders sub_headers = mp.GetSubHeaders("Content-Disposition");

    if (sub_headers.IsEmpty() || !Utilities::iEquals(sub_headers.value, "form-data"))
    {
        return false;
    }

    std::string name = Utilities::GetHeader(sub_headers.headers, "name");

    if (name.empty())
    {
        mp.SetTransferResult(MultiPart::transfer_none);
        return false;
    }

    fdi.name = std::move(name);
    auto filename = Utilities::TryGetHeader(sub_headers.headers, "filename");

    if (filename)
    {
        fdi.data_flag = FormDataItem::is_file_data;
        fdi.filename = std::move(*filename);

        mp.SetTransferResult(MultiPart::transfer_file_memory);
    }
    else
    {
        fdi.data_flag = FormDataItem::is_data;
        mp.SetTransferResult(MultiPart::tranfer_formdata);
    }

    auto content_type = mp.TryGetHeader("Content-Type");

    if (content_type)
    {
        fdi.content_type = std::move(*content_type);
    }

    fdi.data = std::move(mp.GetData());
    return true;
}

void Request::TransferMultiPartsToFormData(UploadFileSaveOptions const& options, std::string const& dir)
{
    for (size_t i = 0; i < _multiparts.size(); ++i)
    {
        MultiPart& part = _multiparts[i];
        FormDataItem fd_item;

        if (TransferMultiPartToFormDataItem(part, fd_item))
        {
            if (fd_item.data_flag == FormDataItem::is_file_data
                && !fd_item.filename.empty() && !fd_item.data.empty()
                && !dir.empty())
            {
                std::string ext = fs::extension(fd_item.filename);
                size_t filesize_kb = fd_item.data.size() / 1024;

                if (options.IsNeedSave(ext, filesize_kb))
                {
                    fs::path temporary_file_name = MakeUploadFileTemporaryName(ext, dir);

                    if (!temporary_file_name.empty())
                    {
                        std::string err;

                        if (Utilities::SaveDataToFile(fd_item.data, temporary_file_name, err))
                        {
                            fd_item.data = temporary_file_name.string();
                            fd_item.data_flag = FormDataItem::is_file_temporary_name;
                            part.SetTransferResult(MultiPart::transfer_file_saved);
                        }
                        else
                        {
                            log::Server()->error("Save form-item to template file fail. {}. {}. {}."
                                                 , fd_item.filename, temporary_file_name.native(), err);
                        }
                    }
                }
            }
        }

        _formdata.push_back(std::move(fd_item));
    }
}

std::string Request::Dump() const
{
    std::stringstream ss;

    ss << "url : " << _url.full <<  "\r\n";
    ss << "host : " << _url.host <<  "\r\n";
    ss << "port : " << _url.port <<  "\r\n";
    ss << "path : " << _url.path <<  "\r\n";
    ss << "query : " << _url.query <<  "\r\n";
    ss << "fragment : " << _url.fragment <<  "\r\n";
    ss << "userinfo : " << _url.userinfo <<  "\r\n";

    ss << "url-parameters : \r\n";

    for (auto const& p : _url.parameters)
    {
        ss << p.first << " : " << p.second <<  "\r\n";
    }

    ss << "method : " << this->GetMethodName() <<  "\r\n";

    ss << "headers : \r\n";

    for (auto const& p : _headers)
    {
        ss << p.first << " : " << p.second << "\r\n";
    }

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

    ss << "body : \r\n";
    ss << _body <<  "\r\n";

    ss << "multi-parts : \r\n";

    for (auto const& mp : this->_multiparts)
    {
        for (auto const& h : mp.GetHeaders())
        {
            ss << h.first << " = " << h.second << "\r\n";
        }

        ss << "part data : \r\n";
        ss << mp.GetData() <<  "\r\n";
    }

    ss << "form-data : \r\n";

    for (auto const& fd : this->_formdata)
    {
        ss << "name : " << fd.name << "\r\n"
           << "is-file : " << fd.IsFile() << "\r\n"
           << "data : \r\n" << fd.data << "\r\n";
    }

    ss << "path-parameters : \r\n";

    for (auto const& pp : this->_path_parameters.Items())
    {
        ss << pp.first << " : " << pp.second << "\r\n";
    }

    ss << "cookies : \r\n";

    for (auto const& c : this->_cookies)
    {
        ss << c.first << " : " << c.second << "\r\n";
    }

    return ss.str();
}

}//namespace da4qi4
