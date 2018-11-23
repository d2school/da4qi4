#include "sms_message.hpp"

#include <ctime>
#include <map>
#include <sstream>

#include "utilities/html_utilities.hpp"
#include "utilities/string_utilities.hpp"
#include "utilities/hmac_algo.hpp"

#include "def/debug_def.hpp"

namespace da4qi4
{
namespace alisms
{

/*
    std::string access_key_id = "LTAIUKE0VtpC1M2g";
    std::string secret_access_key = "Qny6V8pQFYzMeIGgawwzHSFXiAKQiN";
    std::string template_code = "SMS_138535019";
    std::string code = "2305";
    std::string template_param = "{\"code\":\"" + code + "\"}";
    alisms::SmsRequest req(access_key_id, "19906017601", "第2学堂", template_code, template_param);

    auto result = req.Make(secret_access_key);

    if (result.first)
    {
        std::cout << result.second << std::endl;
        return 0;
    }
*/

SmsRequest::SmsRequest(std::string const& access_key_id
                       , std::string const& phone_numbers
                       , std::string const& sign_name
                       , std::string const& template_code
                       , const std::string& template_param
                       , ResponseFormat format)
    : AccessKeyId(access_key_id)
    , Format((format == XML) ? "XML" : "JSON")
    , PhoneNumbers(phone_numbers)
    , SignName(sign_name)
    , TemplateCode(template_code)
    , TemplateParam(template_param)
{
    Timestamp.reserve(21);
}

SmsRequest SmsRequest::ForTest()
{
    /* data from ALi document demo */
    SmsRequest req("testId", "15300000001", "阿里云短信测试专用", "SMS_71390007", "{\"customer\":\"test\"}", XML);
    req.SignatureNonce = "45e25e9b-0a6f-4070-8c85-2956eda1b466";
    req.OutId = "123";
    req.Timestamp = "2017-07-12T02:42:19Z";

    return req;
}

std::string POPUrlEncode(std::string const& value)
{
    std::string result = Utilities::UrlEncode(value);
    result = Utilities::ReplaceAll(result, "+", "%20");
    result = Utilities::ReplaceAll(result, "*", "%2A");
    result = Utilities::ReplaceAll(result, "%7E", "~");

    return result;
}

std::pair<bool, std::string> SmsRequest::Make(std::string const& access_secret)
{
    if (access_secret.empty())
    {
        return {false, "Error: Access-Secret is empty."};
    }

#define RETURN_ERROR_IF_EMPTY(VAL) do { if (VAL.empty()) { \
            return {false, "Error : "#VAL" is empty."}; }} while(false)

    RETURN_ERROR_IF_EMPTY(AccessKeyId);
    RETURN_ERROR_IF_EMPTY(Format);
    RETURN_ERROR_IF_EMPTY(SignatureMethod);
    RETURN_ERROR_IF_EMPTY(RegionId);
    RETURN_ERROR_IF_EMPTY(PhoneNumbers);
    RETURN_ERROR_IF_EMPTY(SignName);
    RETURN_ERROR_IF_EMPTY(TemplateCode);

    if (SignatureMethod != "HMAC-SHA1")
    {
        return { false, "Only HMAC-SHA1 support."};
    }

    if (SignatureNonce.empty())
    {
        SignatureNonce = Utilities::GetUUID();
    }

    if (Timestamp.empty())
    {
        std::time_t calendar = std::time(nullptr);
        std::tm gmt = *std::gmtime(&calendar);

        char buf[21];
        std::strftime(buf, 20, "%Y-%m-%dT%H:%M:%S", &gmt); //2017-07-12T02:42:19Z
        buf[19] = 'Z';
        buf[20] = '\0';

        Timestamp = buf;
    }

    std::map<std::string, std::string> _parameters;

#define ADD_SMS_REQUEST_PARAMETER(VAR) do { if(!VAR.empty()) { \
            _parameters[#VAR] = VAR; }} while(false) \

    ADD_SMS_REQUEST_PARAMETER(AccessKeyId);
    ADD_SMS_REQUEST_PARAMETER(Timestamp);
    ADD_SMS_REQUEST_PARAMETER(Format);
    ADD_SMS_REQUEST_PARAMETER(SignatureMethod);
    ADD_SMS_REQUEST_PARAMETER(SignatureVersion);
    ADD_SMS_REQUEST_PARAMETER(SignatureNonce);

    ADD_SMS_REQUEST_PARAMETER(Action);
    ADD_SMS_REQUEST_PARAMETER(Version);
    ADD_SMS_REQUEST_PARAMETER(RegionId);
    ADD_SMS_REQUEST_PARAMETER(PhoneNumbers);
    ADD_SMS_REQUEST_PARAMETER(SignName);
    ADD_SMS_REQUEST_PARAMETER(TemplateCode);
    ADD_SMS_REQUEST_PARAMETER(TemplateParam);
    ADD_SMS_REQUEST_PARAMETER(SmsUpExtendCode);
    ADD_SMS_REQUEST_PARAMETER(OutId);

    std::string sortted_query_string;
    sortted_query_string.reserve(480);
    bool is_first = true;

    for (auto it = _parameters.cbegin(); it != _parameters.cend(); ++it)
    {
        sortted_query_string.append(is_first ? "" : "&")
        .append(POPUrlEncode(it->first))
        .append("=")
        .append(POPUrlEncode(it->second));

        if (is_first)
        {
            is_first = false;
        }
    }

    std::cout << "sortted_query_string : " << sortted_query_string << std::endl;

    std::string data_to_sign;
    data_to_sign.reserve(sortted_query_string.size() + 32);
    data_to_sign.append("GET&").append(POPUrlEncode("/")).append("&")
    .append(POPUrlEncode(sortted_query_string));

    std::cout << "url_to_sign : " << data_to_sign << std::endl;

    std::string key = access_secret + "&";
    std::vector<std::uint8_t> sign_byte = Utilities::HMA_SHA1(key, data_to_sign);

    std::string sign_base64_result = Utilities::Base64Encode(sign_byte.data(), sign_byte.size());

    Signature = POPUrlEncode(sign_base64_result);

    std::stringstream ss;
    ss << "?Signature=" << Signature << "&" << sortted_query_string;
    std::string result = ss.str();
    return {true, result};
}

void from_json(Json const& json, SmsResponse& res)
{
    json.at("Message").get_to(res.Message);
    json.at("RequestId").get_to(res.RequestId);
    json.at("Code").get_to(res.Code);
    json.at("BizId").get_to(res.BizId);
}

namespace
{

std::map<std::string, std::string> alisms_error_code_msg =
{
    {"isp.RAM_PERMISSION_DENY", "RAM权限DENY"},
    {"isv.OUT_OF_SERVICE", "业务停机"},
    {"isv.PRODUCT_UN_SUBSCRIPT", "未开通云通信产品的阿里云客户"},
    {"isv.PRODUCT_UNSUBSCRIBE", "所用服务未开通"},
    {"isv.ACCOUNT_NOT_EXISTS", "账户不存在"},
    {"isv.ACCOUNT_ABNORMAL", "账户异常"},
    {"isv.SMS_TEMPLATE_ILLEGAL", "短信模版不合法"},
    {"isv.SMS_SIGNATURE_ILLEGAL", "短信签名不合法"},
    {"isv.INVALID_PARAMETERS", "参数异常"},
    {"isp.SYSTEM_ERROR", "ISP系统错误"},
    {"isv.MOBILE_NUMBER_ILLEGAL", "非法手机号"},
    {"isv.TEMPLATE_MISSING_PARAMETERS", "手机号码数量超限"},
    {"isv.TEMPLATE_MISSING_PARAMETERS", "模版缺少变量"},
    {"isv.BUSINESS_LIMIT_CONTROL", "业务限流"},
    {"isv.INVALID_JSON_PARAM", "JSON参数不合法，仅支持字符串类型"},
    {"isv.BLACK_KEY_CONTROL_LIMIT", "黑名单管控"},
    {"isv.PARAM_LENGTH_LIMIT", "参数超出长度限制"},
    {"isv.PARAM_NOT_SUPPORT_URL", "不支持URL参数"},
    {"isv.AMOUNT_NOT_ENOUGH", "账户余额不足"},
    {"isv.TEMPLATE_PARAMS_ILLEGAL", "模版变量里包含非法关键字"},
    {"SignatureDoesNotMatch", "Signature加密错误"},
    {"InvalidTimeStamp.Expired", "时间戳错误"},
    {"SignatureNonceUsed", "唯一随机数重复"},
    {"InvalidVersion", "版本号错误"},
    {"InvalidAction.NotFound", "接口地址或接口名错误"},
};

} //namespace

std::string SmsResponse::GetErrorMsg() const
{
    if (Utilities::iEquals(Code, "OK"))
    {
        return Utilities::theEmptyString;
    }

    auto const it = alisms_error_code_msg.find(Code);

    if (it == alisms_error_code_msg.cend())
    {
        return "Unknown SMS send error.";
    }

    return it->second;
}

bool SmsResponse::HasError() const
{
    return !Utilities::iEquals(Code, "OK");
}

std::string SmsResponse::Parse(std::string const& response)
{
    try
    {
        Json json = Json::parse(response);
        from_json(json, *this);
        return Utilities::theEmptyString;
    }
    catch (std::exception const& e)
    {
        return e.what();
    }
}

} //namespace alisms
} // namespace da4qi4
