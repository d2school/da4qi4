#include "response.hpp"

#include <cstring>
#include <map>

namespace da4qi4
{

char const* const continue_100_response = "HTTP/1.1 100 Continue\r\nContent-Length:0\r\n\r\n";
size_t len_continue_100_response = 43; //strlen(continue_100_response)

struct HttpStatusSummary
{
    friend char const* HttpStatusSummaryGetter(int);
private:
    HttpStatusSummary()
    {
        init();
    }
    
    void init();
    std::map<int, char const*> _map;
};

#define ADD_SUMMARY(CODE, SUMMARY) _map[CODE] = #SUMMARY

void HttpStatusSummary::init()
{
    ADD_SUMMARY(HTTP_STATUS_CONTINUE,                        Continue)                        ;
    ADD_SUMMARY(HTTP_STATUS_SWITCHING_PROTOCOLS,             Switching Protocols)             ;
    ADD_SUMMARY(HTTP_STATUS_PROCESSING,                      Processing)                      ;
    ADD_SUMMARY(HTTP_STATUS_OK,                              OK)                              ;
    ADD_SUMMARY(HTTP_STATUS_CREATED,                         Created)                         ;
    ADD_SUMMARY(HTTP_STATUS_ACCEPTED,                        Accepted)                        ;
    ADD_SUMMARY(HTTP_STATUS_NON_AUTHORITATIVE_INFORMATION,   Non - Authoritative Information)   ;
    ADD_SUMMARY(HTTP_STATUS_NO_CONTENT,                      No Content)                      ;
    ADD_SUMMARY(HTTP_STATUS_RESET_CONTENT,                   Reset Content)                   ;
    ADD_SUMMARY(HTTP_STATUS_PARTIAL_CONTENT,                 Partial Content)                 ;
    ADD_SUMMARY(HTTP_STATUS_MULTI_STATUS,                    Multi - Status)                    ;
    ADD_SUMMARY(HTTP_STATUS_ALREADY_REPORTED,                Already Reported)                ;
    ADD_SUMMARY(HTTP_STATUS_IM_USED,                         IM Used)                         ;
    ADD_SUMMARY(HTTP_STATUS_MULTIPLE_CHOICES,                Multiple Choices)                ;
    ADD_SUMMARY(HTTP_STATUS_MOVED_PERMANENTLY,               Moved Permanently)               ;
    ADD_SUMMARY(HTTP_STATUS_FOUND,                           Found)                           ;
    ADD_SUMMARY(HTTP_STATUS_SEE_OTHER,                       See Other)                       ;
    ADD_SUMMARY(HTTP_STATUS_NOT_MODIFIED,                    Not Modified)                    ;
    ADD_SUMMARY(HTTP_STATUS_USE_PROXY,                       Use Proxy)                       ;
    ADD_SUMMARY(HTTP_STATUS_TEMPORARY_REDIRECT,              Temporary Redirect)              ;
    ADD_SUMMARY(HTTP_STATUS_PERMANENT_REDIRECT,              Permanent Redirect)              ;
    ADD_SUMMARY(HTTP_STATUS_BAD_REQUEST,                     Bad Request)                     ;
    ADD_SUMMARY(HTTP_STATUS_UNAUTHORIZED,                    Unauthorized)                    ;
    ADD_SUMMARY(HTTP_STATUS_PAYMENT_REQUIRED,                Payment Required)                ;
    ADD_SUMMARY(HTTP_STATUS_FORBIDDEN,                       Forbidden)                       ;
    ADD_SUMMARY(HTTP_STATUS_NOT_FOUND,                       Not Found)                       ;
    ADD_SUMMARY(HTTP_STATUS_METHOD_NOT_ALLOWED,              Method Not Allowed)              ;
    ADD_SUMMARY(HTTP_STATUS_NOT_ACCEPTABLE,                  Not Acceptable)                  ;
    ADD_SUMMARY(HTTP_STATUS_PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   ;
    ADD_SUMMARY(HTTP_STATUS_REQUEST_TIMEOUT,                 Request Timeout)                 ;
    ADD_SUMMARY(HTTP_STATUS_CONFLICT,                        Conflict)                        ;
    ADD_SUMMARY(HTTP_STATUS_GONE,                            Gone)                            ;
    ADD_SUMMARY(HTTP_STATUS_LENGTH_REQUIRED,                 Length Required)                 ;
    ADD_SUMMARY(HTTP_STATUS_PRECONDITION_FAILED,             Precondition Failed)             ;
    ADD_SUMMARY(HTTP_STATUS_PAYLOAD_TOO_LARGE,               Payload Too Large)               ;
    ADD_SUMMARY(HTTP_STATUS_URI_TOO_LONG,                    URI Too Long)                    ;
    ADD_SUMMARY(HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          ;
    ADD_SUMMARY(HTTP_STATUS_RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           ;
    ADD_SUMMARY(HTTP_STATUS_EXPECTATION_FAILED,              Expectation Failed)              ;
    ADD_SUMMARY(HTTP_STATUS_MISDIRECTED_REQUEST,             Misdirected Request)             ;
    ADD_SUMMARY(HTTP_STATUS_UNPROCESSABLE_ENTITY,            Unprocessable Entity)            ;
    ADD_SUMMARY(HTTP_STATUS_LOCKED,                          Locked)                          ;
    ADD_SUMMARY(HTTP_STATUS_FAILED_DEPENDENCY,               Failed Dependency)               ;
    ADD_SUMMARY(HTTP_STATUS_UPGRADE_REQUIRED,                Upgrade Required)                ;
    ADD_SUMMARY(HTTP_STATUS_PRECONDITION_REQUIRED,           Precondition Required)           ;
    ADD_SUMMARY(HTTP_STATUS_TOO_MANY_REQUESTS,               Too Many Requests)               ;
    ADD_SUMMARY(HTTP_STATUS_REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) ;
    ADD_SUMMARY(HTTP_STATUS_UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   ;
    ADD_SUMMARY(HTTP_STATUS_INTERNAL_SERVER_ERROR,           Internal Server Error)           ;
    ADD_SUMMARY(HTTP_STATUS_NOT_IMPLEMENTED,                 Not Implemented)                 ;
    ADD_SUMMARY(HTTP_STATUS_BAD_GATEWAY,                     Bad Gateway)                     ;
    ADD_SUMMARY(HTTP_STATUS_SERVICE_UNAVAILABLE,             Service Unavailable)             ;
    ADD_SUMMARY(HTTP_STATUS_GATEWAY_TIMEOUT,                 Gateway Timeout)                 ;
    ADD_SUMMARY(HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      ;
    ADD_SUMMARY(HTTP_STATUS_VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         ;
    ADD_SUMMARY(HTTP_STATUS_INSUFFICIENT_STORAGE,            Insufficient Storage)            ;
    ADD_SUMMARY(HTTP_STATUS_LOOP_DETECTED,                   Loop Detected)                   ;
    ADD_SUMMARY(HTTP_STATUS_NOT_EXTENDED,                    Not Extended)                    ;
    ADD_SUMMARY(HTTP_STATUS_NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) ;
}

char const* HttpStatusSummaryGetter(int code)
{
    static HttpStatusSummary rsm;
    std::map<int, char const*>::const_iterator cit  = rsm._map.find(code);
    return (cit == rsm._map.cend()) ? "" : cit->second;
}


Response::Response()
{
    _headers["Server"] = "da4qi4/0.8.0";
}

std::ostream& operator << (std::ostream& os, Response const& res)
{
    int code = res.GetStatusCode();
    char const* summary = HttpStatusSummaryGetter(code);
    
    os << "HTTP/1.1 " << code << ' ' << summary << "\r\n";
    
    for (auto const& h : res.GetHeaders())
    {
        os << h.first << ":" << h.second << "\r\n";
    }
    
    os << "Content-Length:" << res.GetBody().size() << "\r\n\r\n";
    os << res.GetBody();
    
    return os;
}


}
