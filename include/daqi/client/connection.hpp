#ifndef DAQI_CLIENT_CONNECTION_HPP
#define DAQI_CLIENT_CONNECTION_HPP

#include <memory>
#include <boost/asio/ssl/stream.hpp>

#include "http-parser/http_parser.h"

#include "daqi/def/def.hpp"
#include "daqi/def/boost_def.hpp"
#include "daqi/def/asio_def.hpp"
#include "daqi/utilities/string_utilities.hpp"

namespace da4qi4
{
namespace Client
{

namespace detail
{

using ReadBuffer = std::array<char, 1024 * 2>;

struct SocketBase
{
    virtual ~SocketBase() = 0;

    virtual void async_connect(Tcp::endpoint const&, std::function<void (errorcode const&)>) = 0;

    virtual void async_read_some(ReadBuffer&, std::function<void (errorcode const&, std::size_t)>) = 0;

    virtual void async_write(char* const, std::size_t, std::function<void (errorcode const&, std::size_t)>) = 0;

    virtual void close(errorcode& ec) = 0;

    virtual Tcp::socket& get_socket() = 0;
};

struct Socket : SocketBase
{
    Socket(IOC& ioc)
        : _socket(ioc)
    {
    }

    ~Socket() override;
    Tcp::socket& get_socket() override;

    void async_connect(Tcp::endpoint const& ep,
                       std::function<void (errorcode const& ec)> on_connect) override;
    void async_read_some(ReadBuffer& read_buffer,
                         std::function<void (errorcode const&, std::size_t)> on_read) override;
    void async_write(char* const write_buffer, std::size_t size,
                     std::function<void (errorcode const&, std::size_t)> on_write) override;

    void close(errorcode& ec) override;

private:
    Tcp::socket _socket;
};

struct SocketWithSSL : SocketBase
{
    SocketWithSSL(IOC& ioc, boost::asio::ssl::context& ssl_ctx)
        : _stream(ioc, ssl_ctx)
    {
    }

    ~SocketWithSSL() override;

    void async_connect(Tcp::endpoint const& ep,
                       std::function<void (errorcode const& ec)> on_connect) override;
    void async_read_some(ReadBuffer& read_buffer,
                         std::function<void (errorcode const&, std::size_t)> on_read) override;
    void async_write(char* const write_buffer, std::size_t size,
                     std::function<void (errorcode const&, std::size_t)> on_write) override;

    Tcp::socket& get_socket() override;

    void close(errorcode& ec) override;

private:
    boost::asio::ssl::stream<Tcp::socket> _stream;
};

} // namespace detail

class Connection
    : public std::enable_shared_from_this<Connection>
{
public:
    Connection(IOC& ioc, std::string const& server);
    Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    ~Connection();

public:
    enum class Error {on_none,
                      on_resolver,
                      on_connect,
                      on_write,
                      on_read,
                      on_close
                     };

public:
    Connection& SetMethod(std::string const& method, std::string const& uri);
    Connection& SetHTTPVersion(std::string const& version);

    Connection& ResetHeaders();
    Connection& AddHeader(std::string const& name, std::string const& value);

    enum class BodySetAction {none, reset_content_length};
    Connection& SetBody(std::string body
                        , BodySetAction action = BodySetAction::none);

    using NotifyFunction = std::function<void (errorcode const&)>;

public:
    void Connect(NotifyFunction notify);

    void Write(NotifyFunction notify);
    void Write(std::string const& body, NotifyFunction notify)
    {
        SetBody(body);
        Write(notify);
    }

    void Read(NotifyFunction notify);

    void Reset();
    void Close();

    enum class ActionAfterRequest { keep_connection, close_connection };
    void Request(NotifyFunction notify, ActionAfterRequest action = ActionAfterRequest::keep_connection);

public:
    IOC& GetIOC()
    {
        return _ioc;
    }

    bool IsWithSSL() const
    {
        return _with_ssl;
    }

    std::string const& GetServer() const
    {
        return  _server;
    }

    Tcp::endpoint const& GetServerAddress() const
    {
        return _server_endpoint;
    }

    std::string const& GetRequestMedthod() const
    {
        return _method;
    }
    std::string const& GetRequestURI() const
    {
        return _uri;
    }
    std::string const& GetRequestHTTPVersion() const
    {
        return  _http_version;
    }

    ICHeaders const& GetRequestHeaders() const
    {
        return _request_headers;
    }
    std::string const& GetRequestBody() const
    {
        return _request_body;
    }

    ICHeaders const& GetResponseHeaders() const
    {
        return _response_headers;
    }
    std::string const& GetResponseBody() const
    {
        return _response_body;
    }

    Error GetError() const
    {
        return this->_error;
    }

    std::string const& GetErrorMessage() const
    {
        return this->_error_msg;
    }

    unsigned int GetResponseStatusCode() const
    {
        return (_parser != nullptr) ? _parser->status_code : 0;
    }

    std::string const& GetResponseStatus() const
    {
        return _status_buffer;
    }

private:
    void do_resolver(NotifyFunction notify);
    void do_connect(NotifyFunction notify);

    void do_write(NotifyFunction notify);
    void do_read(NotifyFunction notify);

    void do_close();

private:
    void init_parser();
    void init_parser_setting();

    static int on_header_field(http_parser* parser, char const* at, size_t length);
    static int on_header_value(http_parser* parser, char const* at, size_t length);
    static int on_headers_complete(http_parser* parser);

    void try_commit_reading_response_header();

    static int on_message_begin(http_parser* parser);
    static int on_message_complete(http_parser* parser);

    static int on_status(http_parser* parser, char const* at, size_t length);
    static int on_body(http_parser* parser, char const* at, size_t length);

private:
    void reset();

private:
    bool _with_ssl;
    IOC& _ioc;
    Tcp::resolver _resolver;
    detail::SocketBase* _socket_ptr;

    size_t _ioc_index;
    detail::ReadBuffer _read_buffer;

    bool _is_connected = false;
private:
    http_parser* _parser;
    http_parser_settings _parser_setting;

    enum ReadingHeaderPart {header_none_part, header_field_part, header_value_part};
    ReadingHeaderPart  _reading_header_part = header_none_part;
    std::string _reading_header_field;
    std::string _reading_header_value;

    enum ReadCompletePart {read_none_complete,
                           read_header_complete,
                           read_message_complete
                          };

    ReadCompletePart _read_complete = read_none_complete;

private:
    std::string _server;
    Tcp::endpoint _server_endpoint;

    std::string _method;
    std::string _uri;
    std::string _http_version;

    ICHeaders _request_headers;
    std::string _request_body;

    std::string _url_buffer;
    std::string _status_buffer;
    ICHeaders _response_headers;
    std::string _response_body;

private:
    Error _error;
    std::string _error_msg;

    NotifyFunction _notify;
};

} // namespace Client
} // namespace da4qi4

#endif // DAQI_CLIENT_CONNECTION_HPP
