#ifndef DAQI_CONNECTION_HPP
#define DAQI_CONNECTION_HPP

#include <memory>

#include "http-parser/http_parser.h"
#include "multipart-parser/multipart_parser.h"

#include "daqi/def/asio_def.hpp"

#include "daqi/request.hpp"
#include "daqi/response.hpp"
#include "daqi/handler.hpp"

namespace da4qi4
{

class Application;

class Connection
    : public std::enable_shared_from_this<Connection>
{
    explicit Connection(IOC& ioc, size_t ioc_index);
public:
    static ConnectionPtr Create(IOC& ioc, size_t ioc_index)
    {
        return ConnectionPtr(new Connection(ioc, ioc_index));
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    ~Connection();

    void StartRead();
    void StartWrite();
    void Stop();

public:
    Request const& GetRequest() const
    {
        return _request;
    }
    Request& GetRequest()
    {
        return _request;
    }

    Response& GetResponse()
    {
        return _response;
    }
    Response const& GetResponse() const
    {
        return _response;
    }

    bool HasApplication() const
    {
        return _app != nullptr;
    }
    std::shared_ptr<Application> GetApplication();

    size_t GetIOContextIndex() const
    {
        return _ioc_index;
    }

public:
    Tcp::socket& GetSocket()
    {
        return _socket;
    }

private:
    void do_read();
    void do_write();
    void do_close();

    void do_write_header_for_chunked();
    void do_write_next_chunked_body(clock_t start_wait_clock = 0);
    void do_write_chunked_body_finished(boost::system::error_code const& ec, size_t bytes_transferred);

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
    void update_request_after_header_parsed();
    void update_request_url_after_app_resolve();

    void try_commit_reading_request_header();
    void process_100_continue_request();
    void process_app_no_found();
    void process_too_large_size_upload();

    void try_fix_multipart_bad_request_without_boundary();
    void try_init_multipart_parser();

    enum MultpartParseStatus { mp_cannot_init = -1, mp_parsing = 0,  mp_parse_fail = 1};
    MultpartParseStatus do_multipart_parse();

    bool try_route_application();

private:
    void prepare_response_headers_about_connection();
    void prepare_response_headers_for_chunked_write();
    void reset();

private:
    Tcp::socket _socket;
    size_t _ioc_index;
    std::array<char, 1024 * 4> _buffer;

private:
    http_parser* _parser;
    http_parser_settings _parser_setting;

    std::string  _url_buffer;

    enum ReadingHeaderPart {header_none_part, header_field_part, header_value_part};
    ReadingHeaderPart  _reading_header_part = header_none_part;
    std::string _reading_header_field;
    std::string _reading_header_value;

    std::string _body_buffer;

    enum ReadCompletePart {read_none_complete,
                           read_header_complete,
                           read_message_complete
                          };

    ReadCompletePart _read_complete = read_none_complete;

private:
    enum MultipartParsePart {mp_parse_none,
                             mp_parse_header_field, mp_parse_header_value, mp_parse_headers_complete,
                             mp_parse_data_begin, mp_parse_data, mp_parse_data_end, mp_parse_body_end
                            };

    void init_multipart_parser(std::string const& boundary);
    enum mp_free_flag  {will_free_mp_setting = 1, will_free_mp_parser = 2, will_free_mp_both = 3 };
    void free_multipart_parser(mp_free_flag flag = will_free_mp_both);

    multipart_parser_settings* _mp_parser_setting = nullptr;
    multipart_parser* _mp_parser = nullptr;
    MultiPart _reading_part;
    std::string _reading_part_buffer;
    MultipartParsePart _multipart_parse_part = mp_parse_none;

    boost::asio::streambuf _write_buffer;
    std::string _current_chunked_body_buffer;
private:
    Request _request;
    Response _response;
    std::shared_ptr<Application> _app = nullptr;
};

} //namespace da4qi4

#endif // DAQI_CONNECTION_HPP
