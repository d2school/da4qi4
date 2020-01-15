#ifndef DAQI_CLIENT_CONNECTION_HPP
#define DAQI_CLIENT_CONNECTION_HPP

#include <memory>
#include <boost/asio/ssl/stream.hpp>

#include "llhttp/llhttp.h"

#include "daqi/def/def.hpp"

#include "daqi/net-detail/net_detail_client.hpp"

#include "daqi/utilities/http_utilities.hpp"
#include "daqi/utilities/string_utilities.hpp"
#include "daqi/utilities/html_utilities.hpp"

namespace da4qi4
{
namespace Client
{

class Connection final
{
    typedef llhttp_t http_parser;
    typedef llhttp_settings_t http_parser_settings;

    Connection(IOC& ioc, std::string const& server);
    Connection(IOC& ioc, std::string const& server, std::string const& service);
    Connection(IOC& ioc, std::string const& server, unsigned short port);

    Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server);
    Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
               , std::string const& service);
    Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
               , unsigned short port);

public:
    typedef std::unique_ptr<Connection> Ptr;

    static Ptr Create(IOC& ioc, std::string const& server)
    {
        return Ptr(new Connection(ioc, server));
    }

    static Ptr Create(IOC& ioc, std::string const& server, std::string const& service)
    {
        return Ptr(new Connection(ioc, server, service));
    }

    static Ptr Create(IOC& ioc, std::string const& server, unsigned short port)
    {
        return Ptr(new Connection(ioc, server, port));
    }

    static Ptr Create(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server)
    {
        return Ptr(new Connection(ioc, ctx, server));
    }

    static Ptr Create(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
                      , std::string const& service)
    {
        return Ptr(new Connection(ioc, ctx, server, service));
    }

    Ptr Create(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
               , unsigned short port)
    {
        return Ptr(new Connection(ioc, ctx, server, port));
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    ~Connection();

public:
    enum class Error {on_none = 0,
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

    enum class ActionAfterRequest { keep_connection, close_connection };
    void Request(NotifyFunction notify
                 , ActionAfterRequest action = ActionAfterRequest::keep_connection);
public:
    bool ConnectSync();
    bool WriteSync(std::size_t& bytes_transferred);
    bool ReadSync(std::size_t& bytes_transferred);
    bool RequestSync(std::size_t& bytes_wrote, std::size_t& bytes_read
                     , ActionAfterRequest action = ActionAfterRequest::keep_connection);

    bool RequestSync(ActionAfterRequest action = ActionAfterRequest::keep_connection)
    {
        std::size_t w(0), r(0);
        return RequestSync(w, r, action);
    }

public:
    void Reset();
    void Close();

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

    bool HasError() const
    {
        return _error != Error::on_none;
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
    std::string make_request_buffer();

    void do_resolver(NotifyFunction notify);
    void do_connect(NotifyFunction notify);

    void do_write(NotifyFunction notify);
    void do_read(NotifyFunction notify);

    errorcode do_resolver();
    errorcode do_connect();
    errorcode do_write(std::size_t& bytes_transferred);
    errorcode do_read(std::size_t& bytes_transferred);

    void do_close();

private:
    void init();
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
    net_detail::SocketBase* _socket_ptr;

    size_t _ioc_index;
    net_detail::ReadBuffer _read_buffer;

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
    std::string _service;
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
