#ifndef Connection_HPP
#define Connection_HPP

#include <memory>

#include "asio_def.hpp"
#include "request.hpp"

#include "http-parser/http_parser.h"
#include "multipart-parser/multipart_parser.h"

namespace da4qi4
{

class Connection
  : public std::enable_shared_from_this<Connection>
{
public:
    using Ptr = std::shared_ptr<Connection>;
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    explicit Connection(Tcp::socket socket);
    ~Connection();

    void Start();
    void Stop();

private:
    void do_read();
    void do_write();
    void do_close();

private:
    Tcp::socket _socket;
    std::array<char, 1024> _buffer;

private:
    void init_parser();
    void init_parser_setting();

    static int on_header_field(http_parser* parser, char const* at, size_t length);
    static int on_header_value(http_parser* parser, char const* at, size_t length);
    static int on_headers_complete(http_parser* parser);

    static int on_message_begin(http_parser* parser);
    static int on_message_complete(http_parser* parser);

    static int on_url(http_parser* parser, char const* at, size_t length);
    static int on_body(http_parser* parser, char const* at, size_t length);

private:
    static int on_multipart_header_field(multipart_parser* parser, char const* at, size_t length);
    static int on_multipart_header_value(multipart_parser* parser, char const* at, size_t length);
    static int on_multipart_headers_complete(multipart_parser* parser);
    
    static int on_multipart_data_begin(multipart_parser* parser);
    static int on_multipart_data(multipart_parser* parser, char const* at, size_t length);
    static int on_multipart_data_end(multipart_parser* parser);
    static int on_multipart_body_end(multipart_parser* parser);
    
private:
    http_parser* _parser;
    http_parser_settings _parser_setting;

    std::string  _url;

    enum ReadingHeaderPart {header_none_part, header_field_part, header_value_part};
    ReadingHeaderPart  _reading_header_part = header_none_part;
    std::string _reading_header_field, _reading_header_value;

    std::string _body;

    enum ReadCompletePart {read_none_complete, read_header_complete, read_message_complete};
    ReadCompletePart _read_complete = read_none_complete;

private:
    void init_multipart_parser(std::string const& boundary);
    void free_multipart_parser();
    multipart_parser_settings* _mp_parser_setting = nullptr;
    multipart_parser* _mp_parser = nullptr;

private:
    Request _request;
};

} //namespace da4qi4

#endif // Connection_HPP
