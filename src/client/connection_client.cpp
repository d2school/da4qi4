#include "daqi/client/connection_client.hpp"

#include "daqi/utilities/asio_utilities.hpp"

#include <iostream>

namespace da4qi4
{
namespace Client
{

namespace detail
{

SocketBase::~SocketBase() {};

Socket::~Socket()
{
}

Tcp::socket& Socket::get_socket()
{
    return _socket;
}

void Socket::async_connect(Tcp::endpoint const& ep,
                           std::function<void (errorcode const& ec)> on_connect)
{
    _socket.async_connect(ep, on_connect);
}

void Socket::async_read_some(ReadBuffer& read_buffer,
                             std::function<void (errorcode const&, std::size_t)> on_read)
{
    _socket.async_read_some(boost::asio::buffer(read_buffer), on_read);
}

void Socket::async_write(char* const write_buffer, std::size_t size,
                         std::function<void (errorcode const&, std::size_t)> on_write)
{
    boost::asio::async_write(_socket, boost::asio::buffer(write_buffer, size), on_write);
}

void Socket::close(errorcode& ec)
{
    _socket.shutdown(boost::asio::socket_base::shutdown_both, ec);
    _socket.close(ec);
}

SocketWithSSL::~SocketWithSSL()
{
}

void SocketWithSSL::async_connect(Tcp::endpoint const& ep,
                                  std::function<void (errorcode const& ec)> on_connect)
{
    _stream.lowest_layer().async_connect(ep, [this, ep, on_connect](errorcode const & ec)
    {
        if (ec)
        {
            on_connect(ec);
            return;
        }

        _stream.set_verify_mode(boost::asio::ssl::verify_none);
        _stream.async_handshake(boost::asio::ssl::stream_base::client, on_connect);
    });
}

void SocketWithSSL::async_read_some(ReadBuffer& read_buffer,
                                    std::function<void (errorcode const&, std::size_t)> on_read)
{
    _stream.async_read_some(boost::asio::buffer(read_buffer), on_read);
}

void SocketWithSSL::async_write(char* const write_buffer, std::size_t size,
                                std::function<void (errorcode const&, std::size_t)> on_write)
{
    boost::asio::async_write(_stream, boost::asio::buffer(write_buffer, size), on_write);
}

void SocketWithSSL::close(errorcode& ec)
{
    _stream.shutdown(ec);
    _stream.next_layer().close(ec);
}

Tcp::socket& SocketWithSSL::get_socket()
{
    return _stream.next_layer();
}

} // namespace detail

namespace
{
std::string port_to_service(unsigned short port)
{
    return std::to_string(port);
}
}

Connection::Connection(IOC& ioc, std::string const& server)
    : _with_ssl(false), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new detail::Socket(ioc)),
      _parser(new http_parser), _server(server),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, std::string const& server, std::string const& service)
    : _with_ssl(false), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new detail::Socket(ioc)),
      _parser(new http_parser), _server(server), _service(service),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, std::string const& server, unsigned short port)
    : _with_ssl(false), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new detail::Socket(ioc)),
      _parser(new http_parser), _server(server), _service(port_to_service(port)),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, boost::asio::ssl::context& ctx, const std::string& server)
    : _with_ssl(true), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new detail::SocketWithSSL(ioc, ctx)),
      _parser(new http_parser), _server(server),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
                       , const std::string& service)
    : _with_ssl(true), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new detail::SocketWithSSL(ioc, ctx)),
      _parser(new http_parser), _server(server), _service(service),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
                       , unsigned short port)
    : _with_ssl(true), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new detail::SocketWithSSL(ioc, ctx)),
      _parser(new http_parser), _server(server), _service(port_to_service(port)),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::~Connection()
{
    do_close();

    delete _parser;
    delete _socket_ptr;
}

void Connection::init()
{
    this->init_parser();
    this->init_parser_setting();

    ResetHeaders();
}

void Connection::Reset()
{
    reset();
}

void Connection::Close()
{
    do_close();
}

void Connection::do_close()
{
    if (!_is_connected)
    {
        return;
    }

    errorcode ec;
    _socket_ptr->close(ec);

    if (ec)
    {
        _error = Error::on_close;
        _error_msg = "Socket close exception. " + ec.message();
    }

    _is_connected = false;
}

void Connection::init_parser()
{
    http_parser_init(_parser, HTTP_RESPONSE);
    _parser->data = this;
}

void Connection::init_parser_setting()
{
    http_parser_settings_init(&_parser_setting);

    _parser_setting.on_message_begin = &Connection::on_message_begin;
    _parser_setting.on_message_complete = &Connection::on_message_complete;
    _parser_setting.on_headers_complete = &Connection::on_headers_complete;
    _parser_setting.on_header_field = &Connection::on_header_field;
    _parser_setting.on_header_value = &Connection::on_header_value;
    _parser_setting.on_status = &Connection::on_status;
    _parser_setting.on_body = &Connection::on_body;
}

Connection& Connection::SetMethod(std::string const& method, std::string const& uri)
{
    this->_method = method;
    this->_uri = uri.empty() ? "/" : uri;
    return *this;
}

Connection& Connection::SetHTTPVersion(std::string const& version)
{
    this->_http_version = version;
    return *this;
}

Connection& Connection::ResetHeaders()
{
    _request_headers.clear();
    std::string user_agent_with_version = the_daqi_name + std::string("/") + the_daqi_version;
    _request_headers.insert(std::make_pair("User-Agent", user_agent_with_version));
    return *this;
}

Connection& Connection::AddHeader(std::string const& name, std::string const& value)
{
    auto it = _request_headers.find(name);

    if (it != _request_headers.end())
    {
        it->second = value;
    }
    else
    {
        _request_headers.insert(std::make_pair(name, value));
    }

    return *this;
}

Connection& Connection::SetBody(std::string body, BodySetAction action)
{
    if (action == BodySetAction::reset_content_length)
    {
        AddHeader("Content-Length", std::to_string(body.length()));
    }

    this->_request_body = std::move(body);
    return *this;
}

void Connection::do_resolver(NotifyFunction notify)
{
    static std::string const http_services [] = {"http", "https"};

    Utilities::from_host(_server
                         , (_service.empty() ? http_services[_with_ssl ? 1 : 0] : _service)
                         , _resolver
                         , [this, notify](errorcode const & ec, Tcp::resolver::results_type results)
    {
        if (ec)
        {
            _error = Error::on_resolver;
            _error_msg = "Resolver host address exception.";
            notify(ec);
            return;
        }

        if (results.empty())
        {
            _error = Error::on_resolver;
            _error_msg = "Resolver host address fail.";
            notify(ec);
            return;
        }

        this->_server_endpoint = *results.cbegin();

        this->do_connect(notify);
    });
}

void Connection::do_connect(NotifyFunction notify)
{
    assert(_server_endpoint.port() != 0);

    if (_is_connected)
    {
        notify(errorcode());
        return;
    }

    _socket_ptr->async_connect(this->_server_endpoint, [this, notify](errorcode const & ec)
    {
        if (ec)
        {
            _error = Error::on_connect;
            _error_msg = "Connect to server fail. ";
        }
        else
        {
            _is_connected = true;
            reset();
        }

        notify(ec);
    });
}

void Connection::do_write(NotifyFunction notify)
{
    std::stringstream os;
    os << _method << ' ' << _uri << " HTTP/" << _http_version << "\r\n"
       << "Host:" << _server << "\r\n";

    for (auto kv : _request_headers)
    {
        os << kv.first << ":" << kv.second << "\r\n";
    }

    os << "\r\n";

    if (!_request_body.empty())
    {
        os << _request_body;
    }

    std::string request_buffer = os.str();

    _socket_ptr->async_write(request_buffer.data(), request_buffer.size(),
                             [this, notify](errorcode const & ec, size_t /*bytes_transferred*/)
    {
        if (ec)
        {
            _error = Error::on_write;
            _error_msg = "Write to server fail.";
        }

        notify(ec);
    });
}

void Connection::do_read(NotifyFunction notify)
{
    _socket_ptr->async_read_some(_read_buffer, [this, notify](errorcode const & ec,
                                                              std::size_t bytes_transferred)
    {
        if (ec)
        {
            _error = Error::on_read;
            _error_msg = "Read from server fail.";
            notify(ec);
            return;
        }

        auto parsed = http_parser_execute(_parser, &_parser_setting, _read_buffer.data()
                                                                         , bytes_transferred);

        if (parsed != bytes_transferred || _parser->http_errno)
        {
            _error = Error::on_read;
            _error_msg = "Parse response content error. "
                         + std::to_string(_parser->http_errno) + ".";

            notify(ec);
            return;
        }

        if (_read_complete != read_message_complete)
        {
            do_read(notify);
            return;
        }

        notify(ec);
    });
}

void Connection::try_commit_reading_response_header()
{
    bool have_a_uncommit_header = _reading_header_part == Connection::header_value_part
                                  && !_reading_header_field.empty();

    if (have_a_uncommit_header)
    {
        this->_response_headers.insert(std::make_pair(
                                                   std::move(_reading_header_field),
                                                   std::move(_reading_header_value))
                                      );
        _reading_header_part = Connection::header_none_part;
    }
}

int Connection::on_headers_complete(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->try_commit_reading_response_header();

    cnt->_read_complete = Connection::read_header_complete;

    return 0;
}

int Connection::on_message_begin(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_read_complete = Connection::read_none_complete;
    cnt->_response_body.clear();

    return 0;
}

int Connection::on_message_complete(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_read_complete = Connection::read_message_complete;

    return 0;
}

int Connection::on_header_field(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->try_commit_reading_response_header();

    if (length > 0)
    {
        cnt->_reading_header_part = Connection::header_field_part;
        cnt->_reading_header_field.append(at, length);
    }

    return 0;
}

int Connection::on_header_value(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);

    if (cnt->_reading_header_part == Connection::header_field_part
        && !cnt->_reading_header_value.empty())
    {
        cnt->_reading_header_value.clear();
    }

    if (length > 0)
    {
        cnt->_reading_header_value.append(at, length);
    }

    if (cnt->_reading_header_part != Connection::header_value_part)
    {
        cnt->_reading_header_part = Connection::header_value_part;
    }

    return 0;
}

int Connection::on_status(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_status_buffer.append(at, length);
    return 0;
}

int Connection::on_body(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);

    cnt->_response_body.append(at, length);

    return 0;
}

void Connection::reset()
{
    _error  = Error::on_none;
    _error_msg.clear();

    _status_buffer.clear();
    _url_buffer.clear();
    _response_headers.clear();
    _response_body.clear();

    _reading_header_part = header_none_part;
    _reading_header_field.clear();
    _reading_header_value.clear();
    _read_complete = read_none_complete;
}

void Connection::Connect(NotifyFunction notify)
{
    if (this->_server_endpoint.port() != 0)
    {
        this->do_connect(notify);
        return;
    }

    this->do_resolver([this, notify](errorcode const & ec)
    {
        if (ec)
        {
            notify(ec);
            return;
        }

        this->do_connect(notify);
    });
}

void Connection::Write(NotifyFunction notify)
{
    do_write(notify);
}

void Connection::Read(NotifyFunction notify)
{
    do_read(notify);
}

void Connection::Request(NotifyFunction notify, ActionAfterRequest action)
{
    this->Connect([this, notify, action](errorcode const & ec)
    {
        if (ec)
        {
            notify(ec);
            return;
        }

        do_write([this, notify, action](errorcode const & ec)
        {
            if (ec)
            {
                notify(ec);
                return;
            }

            do_read([this, notify, action](errorcode const & ec)
            {
                notify(ec);

                if (action == ActionAfterRequest::close_connection)
                {
                    this->do_close();
                }
            });
        });
    });
}


} // namespace Client
} // namespace da4qi4
