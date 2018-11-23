#ifndef DAQI_SMS_MESSAGE_HPP
#define DAQI_SMS_MESSAGE_HPP

#include <string>
#include <utility>

#include "def/json_def.hpp"

namespace da4qi4
{
namespace alisms
{

struct SmsRequest
{
    enum ResponseFormat {JSON, XML};

    static std::string GetFormatValue(ResponseFormat fmt)
    {
        return fmt == XML ? "XML" : "JSON";
    }

    SmsRequest() = default;
    SmsRequest(std::string const& access_key_id
               , std::string const& phone_numbers
               , std::string const& sign_name
               , std::string const& template_code
               , std::string const& template_param = ""
               , ResponseFormat format = JSON);

    static SmsRequest ForTest();

    std::string AccessKeyId;
    std::string Timestamp;
    std::string Format = "JSON";
    std::string SignatureMethod = "HMAC-SHA1";
    std::string SignatureVersion = "1.0";
    std::string SignatureNonce;
    std::string Signature;

    std::string Action = "SendSms";
    std::string Version = "2017-05-25";
    std::string RegionId = "cn-hangzhou";
    std::string PhoneNumbers;
    std::string SignName;
    std::string TemplateCode;
    std::string TemplateParam;
    std::string SmsUpExtendCode;
    std::string OutId;

    std::pair<bool, std::string> Make(std::string const& access_secret);

private:
    std::string _request_url;
};

struct SmsResponse
{
    std::string Message;
    std::string RequestId;
    std::string Code;
    std::string BizId;

    bool HasError() const;
    std::string GetErrorMsg() const;

    std::string Parse(std::string const& response);
};

void from_json(Json const& json, SmsResponse& res);

} // namespace alisms
} // namespace da4qi4

#endif // DAQI_SMS_MESSAGE_HPP
