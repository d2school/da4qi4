#include "sms_client.hpp"

#include "def/def.hpp"
#include "def/debug_def.hpp"

#include "utilities/string_utilities.hpp"
#include "utilities/asio_utilities.hpp"


namespace da4qi4
{
namespace alisms
{

std::string SmsClient::head_end_flag = "\r\n\r\n";

std::string aliyun_smsapi_server = "http://dysmsapi.aliyuncs.com/";

std::string get_aliyun_smsapi_host()
{
    std::size_t len = aliyun_smsapi_server.size() - 7 - 1;
    return aliyun_smsapi_server.substr(7, len);
}

SmsClient::SmsClient(IOC& ioc, const std::string& access_key_id,
                     const std::string& access_secret,
                     std::string const& server)
    : _ioc(ioc), _resolver(ioc), _socket(ioc),
      _access_key_id(access_key_id), _access_secret(access_secret), _server(server)
{
}

SmsClient::Ptr SmsClient::Create(IOC& ioc,
                                 std::string const& access_key_id,
                                 std::string const& access_secret, std::string const& server)
{
    return std::shared_ptr<SmsClient>(new SmsClient(ioc, access_key_id, access_secret, server));
}

void SmsClient::Request(OnResponse handler
                        , std::string const& phone_numbers
                        , std::string const& sign_name
                        , std::string const& template_code
                        , std::string const& template_param
                        , SmsRequest::ResponseFormat format
                        , std::string const& sms_up_extend_code
                        , std::string const& out_id
                        , std::string const& region_id)
{
    _error_step = error_on_none;
    _error_msg.clear();

    _request_msg.AccessKeyId = _access_key_id;

    _request_msg.PhoneNumbers = phone_numbers;
    _request_msg.SignName = sign_name;
    _request_msg.TemplateCode = template_code;
    _request_msg.TemplateParam = template_param;
    _request_msg.Format = SmsRequest::GetFormatValue(format);
    _request_msg.SmsUpExtendCode = sms_up_extend_code;
    _request_msg.OutId = out_id;

    _handler = std::forward<OnResponse>(handler);

    if (!region_id.empty())
    {
        _request_msg.RegionId = region_id;
    }

    auto self = shared_from_this();

    Utilities::from_host(get_aliyun_smsapi_host(), "http", _resolver
                         , [ = ](errorcode const & ec,  Tcp::resolver::results_type results)
    {
        if (ec)
        {
            if (_handler)
            {
                _error_step = error_on_resolver;
                _error_msg = "Resolver ALiSMS host fail.";
                _handler(self, ec);
            }

            return;
        }

        if (results.empty())
        {
            auto ec = boost::system::errc::make_error_code(boost::system::errc::address_not_available);

            if (_handler)
            {
                _error_step = error_on_resolver;
                _error_msg = "Resolver ALiSMS host result is empty.";
                _handler(self, ec);
            }

            return;
        }

        start_connect(*results.begin());
    });
}

void SmsClient::start_connect(Tcp::endpoint ep)
{
    auto self = shared_from_this();

    _socket.async_connect(ep, [self, this](errorcode const & ec)
    {
        if (ec)
        {
            if (_handler)
            {
                _error_step = error_on_connect;
                _error_msg = "Connect to ALiSMS server fail.";
                _handler(self, ec);
            }

            return;
        }

        start_write();
    });
}

void SmsClient::start_write()
{
    auto self = shared_from_this();
    auto result = _request_msg.Make(_access_secret);

    if (!result.first)
    {
        auto ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);
        this->_error_msg = result.second;

        if (_handler)
        {
            _error_step = error_on_write;
            _error_msg = "Make request data fail.";
            _handler(self, ec);
        }

        return;
    }

    std::stringstream ss;
    ss << "GET /" << result.second << " HTTP/1.1\r\n"
       << "Host:" << get_aliyun_smsapi_host() << "\r\n"
       << "Accept-Language:zh-cn\r\n"
       << "User-Agent:" << the_daqi_name << "\r\n\r\n";

    _request_buf = ss.str();

    boost::asio::async_write(_socket, boost::asio::buffer(_request_buf),
                             [self, this](errorcode const & ec, std::size_t)
    {
        if (ec)
        {
            if (_handler)
            {
                _error_step = error_on_write;
                _error_msg = "Write request data to ALiSMS server fail.";
                _handler(self, ec);
            }

            return;
        }

        _response_buf.clear();
        start_read_head();
    });
}

void SmsClient::start_read_head()
{
    auto self = shared_from_this();

    _socket.async_read_some(boost::asio::buffer(_read_buf, _read_buf_size_)
                            , [this, self](boost::system::error_code ec, std::size_t size)
    {
        if (ec)
        {
            _error_step = error_on_read;
            _error_msg = "Read response fail.";

            if (_handler)
            {
                _handler(self, ec);
            }

            return;
        }

        if (size == 0)
        {
            start_read_head();
            return;
        }

        _response_buf.append(_read_buf, size);

        std::string::size_type pos = _response_buf.find(head_end_flag);

        if (pos != std::string::npos)
        {
            start_read_body(pos);
        }
        else
        {
            start_read_head();
        }
    });
}

void SmsClient::start_read_body(std::string::size_type head_end_flag_pos)
{
    auto self = shared_from_this();

    _header = _response_buf.substr(0, head_end_flag_pos);
    _body = _response_buf.substr(head_end_flag_pos + head_end_flag_len);
    _response_buf.clear();

    std::string content_length_flag = "content-length:";
    std::size_t matched_count = 0;
    std::string::size_type beg_pos = 0, end_pos = 0;

    for (std::string::size_type i = 0; i < _header.size(); ++i)
    {
        char c = _header[i];

        if (c >= 'A' && c <= 'Z')
        {
            c += ('a' - 'A');
        }

        if (c == content_length_flag[matched_count])
        {
            ++matched_count;

            if (matched_count == content_length_flag.size())
            {
                beg_pos = i + 1;
                break;
            }
        }
        else if (matched_count > 0)
        {
            matched_count = 0;

            if (c == content_length_flag[matched_count])
            {
                matched_count = 1;
            }
        }
    }

    if (matched_count < content_length_flag.size())
    {
        _error_step = error_on_read;
        _error_msg = "Response no found \"content-length\" field.";
        auto ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);

        if (_handler)
        {
            _handler(self, ec);
        }

        return;
    }

    for (; beg_pos < _header.size(); ++beg_pos)
    {
        if (_header[beg_pos] >= '0' && _header[beg_pos] <= '9')
        {
            break;
        }
    }

    if (beg_pos < _header.size())
    {
        for (end_pos = beg_pos + 1; end_pos < _header.size(); ++end_pos)
        {
            if (_header[end_pos] < '0' || _header[end_pos] > '9')
            {
                break;
            }
        }
    }

    if (beg_pos == _header.size() || end_pos == _header.size())
    {
        _error_step = error_on_read;
        _error_msg = "Response no found \"content-length\" value.";
        auto ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);

        if (_handler)
        {
            _handler(self, ec);
        }

        return;
    }

    std::string slen = _header.substr(beg_pos, end_pos - beg_pos);

    _content_len = 0;
    std::stringstream sslen(slen);
    sslen >> _content_len;

    if (_content_len == 0)
    {
        _error_step = error_on_read;
        _error_msg = "Parse response \"content-length\" value fail.";
        auto ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);

        if (_handler)
        {
            _handler(self, ec);
        }

        return;
    }

    int remain = static_cast<int>(_content_len) - static_cast<int>(_body.size());

    if (remain < 0 || remain > 1024 * 500)
    {
        _error_step = error_on_read;
        _error_msg = "Bad value of response \"content-length\".";
        auto ec = boost::system::errc::make_error_code(boost::system::errc::bad_message);

        if (_handler)
        {
            _handler(self, ec);
        }

        return;
    }

    if (remain == 0)
    {
        if (_handler)
        {
            errorcode ec;
            _handler(self, ec);
        }

        return;
    }

    _response_buf.resize(static_cast<std::size_t>(remain));
    boost::asio::async_read(_socket, boost::asio::buffer(_response_buf, static_cast<std::size_t>(remain)),
                            [self, this, remain](errorcode const & ec, std::size_t size)
    {
        if (ec)
        {
            _error_step = error_on_read;
            _error_msg = "Read response body remain " + std::to_string(remain) + " byte fail.";

            if (_handler)
            {
                _handler(self, ec);
            }

            return;
        }

        _body += _response_buf;
        _response_buf.clear();

        assert(static_cast<std::size_t>(remain) == size);

        if (_handler)
        {
            _handler(self, errorcode());
        }
    });
}

} // namespace alisms
} // namespace da4qi4
