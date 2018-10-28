#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>

#include "http-parser/http_parser.h"

#include "def/def.hpp"

namespace da4qi4
{

extern char const* const continue_100_response;
extern size_t len_continue_100_response;

class Response
{
public:
    Response();
    Response(http_status code)
        : _status_code(code)
    {}
    
    int GetStatusCode() const  { return _status_code ; }
    Headers const& GetHeaders() const { return _headers; }
    std::string const& GetBody() const { return _body; }
    
    void SetStatusCode(http_status code) { _status_code = code; }
    void SetBody(std::string&& body) { _body = std::move(body); }
    void AddHeader(std::string const& field, std::string const& value)
    {
        _headers.insert(std::make_pair(field, value));
    }
    
private:
    int _status_code = HTTP_STATUS_OK;
    
    Headers _headers;
    std::string _body;
};

std::ostream& operator << (std::ostream& os, Response const& res);

} //namespace da4qi4

#endif // RESPONSE_HPP
