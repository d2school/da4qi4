#ifndef DAQI_SMS_CLIENT_HPP
#define DAQI_SMS_CLIENT_HPP

#include <string>
#include <memory>

#include "def/boost_def.hpp"
#include "def/asio_def.hpp"

#include "sms_message.hpp"

namespace da4qi4
{
namespace alisms
{

extern std::string aliyun_smsapi_server;

class SmsClient : public std::enable_shared_from_this<SmsClient>
{
    static std::string head_end_flag;
    static size_t const head_end_flag_len = 4;

    SmsClient(IOC& ioc, std::string const& access_key_id, std::string const& access_secret,
              std::string const& server);

public:
    using Ptr = std::shared_ptr<SmsClient>;
    using OnResponse = std::function<void (Ptr client, errorcode const& ec)>;

    SmsClient(SmsClient const&) = delete;

public:
    static Ptr Create(IOC& ioc,
                      std::string const& access_key_id,
                      std::string const& access_secret,
                      std::string const& server = aliyun_smsapi_server);

    void Request(OnResponse handler
                 , std::string const& phone_numbers
                 , std::string const& sign_name
                 , std::string const& template_code
                 , std::string const& template_param
                 , SmsRequest::ResponseFormat format = SmsRequest::JSON
                 , std::string const& sms_up_extend_code = ""
                 , std::string const& out_id = ""
                 , std::string const& region_id = "");

    enum ErrorStep {error_on_none, error_on_resolver, error_on_connect, error_on_write, error_on_read};

    ErrorStep GetErrorStep() const
    {
        return _error_step;
    }

    std::string const& GetErrorMsg() const
    {
        return _error_msg;
    }

    std::string const& GetResponseHeader() const
    {
        return _header;
    }

    std::string const& GetResponseBody() const
    {
        return _body;
    }

    std::size_t GetResponseContentLength() const
    {
        return _content_len;
    }

private:
    void start_connect(Tcp::endpoint ep);
    void start_write();
    void start_read_head();
    void start_read_body(std::string::size_type head_end_flag_pos);

private:
    IOC& _ioc;
    Tcp::resolver _resolver;
    Tcp::socket _socket;

    std::string _access_key_id, _access_secret;
    std::string _server;

    SmsRequest _request_msg;
    std::string _request_buf;
    OnResponse _handler;

    static std::size_t const _read_buf_size_ = 512;
    char _read_buf[_read_buf_size_];
    std::string _response_buf;
    std::string _header;
    std::string _body;
    std::size_t _content_len;

    ErrorStep _error_step = error_on_none;
    std::string _error_msg;
};

} // namespace alisms
} // namespace da4qi4

#endif // DAQI_SMS_CLIENT_HPP
