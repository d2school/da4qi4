#include "daqi/client/connection_client.hpp"

#include "daqi/utilities/asio_utilities.hpp"

#include <iostream>
#include <condition_variable>

namespace da4qi4
{
namespace Client
{

namespace net_detail
{

SocketBase::~SocketBase() {};

struct Socket : SocketBase
{
    Socket(IOC& ioc)
        : _socket(ioc)
    {
    }

    ~Socket() override;
    Tcp::socket& get_socket() override;

    void async_connect(Tcp::endpoint const& ep,
                       SocketConnectionCompletionCallback on_connect) override;

    void async_read_some(ReadBuffer& read_buffer, SocketCompletionCallback on_read) override;

    void async_write(char const* write_buffer, std::size_t size, SocketCompletionCallback on_wrote) override;

    errorcode sync_connect(Tcp::endpoint const& ep) override;
    errorcode sync_read_some(ReadBuffer& read_buffer, std::size_t& bytes_transferred) override;
    errorcode sync_write(char const* write_buffer, std::size_t write_buffer_size
                         , std::size_t& bytes_transferred) override;

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

    Tcp::socket& get_socket() override;

    void async_connect(Tcp::endpoint const& ep,
                       SocketConnectionCompletionCallback on_connect) override;

    void async_read_some(ReadBuffer& read_buffer, SocketCompletionCallback on_read) override;

    void async_write(char const* write_buffer, std::size_t size, SocketCompletionCallback on_wrote) override;

    errorcode sync_connect(Tcp::endpoint const& ep) override;
    errorcode sync_read_some(ReadBuffer& read_buffer, std::size_t& bytes_transferred) override;
    errorcode sync_write(char const* write_buffer, std::size_t write_buffer_size
                         , std::size_t& bytes_transferred) override;

    void close(errorcode& ec) override;

private:
    boost::asio::ssl::stream<Tcp::socket> _stream;
};


Socket::~Socket()
{
}

Tcp::socket& Socket::get_socket()
{
    return _socket;
}

void Socket::async_connect(Tcp::endpoint const& ep,
                           SocketConnectionCompletionCallback on_connect)
{
    _socket.async_connect(ep, on_connect);
}

void Socket::async_read_some(ReadBuffer& read_buffer,
                             SocketCompletionCallback on_read)
{
    _socket.async_read_some(boost::asio::buffer(read_buffer), on_read);
}

void Socket::async_write(char const* write_buffer, std::size_t size,
                         SocketCompletionCallback on_wrote)
{
    boost::asio::async_write(_socket, boost::asio::buffer(write_buffer, size), on_wrote);
}

errorcode Socket::sync_connect(Tcp::endpoint const& ep)
{
    errorcode ec;
    _socket.connect(ep, ec);
    return ec;
}

errorcode Socket::sync_read_some(ReadBuffer& read_buffer, std::size_t& bytes_transferred)
{
    errorcode ec;
    bytes_transferred = _socket.read_some(boost::asio::buffer(read_buffer), ec);
    return ec;
}

errorcode Socket::sync_write(char const* write_buffer, std::size_t write_buffer_size
                             , std::size_t& bytes_transferred)
{
    errorcode ec;
    bytes_transferred = boost::asio::write(_socket
                                           , boost::asio::buffer(write_buffer, write_buffer_size)
                                           , ec);
    return ec;
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
                                  SocketConnectionCompletionCallback on_connect)
{
    _stream.lowest_layer().async_connect(ep, [this, on_connect](errorcode const & ec)
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
                                    SocketCompletionCallback on_read)
{
    _stream.async_read_some(boost::asio::buffer(read_buffer), on_read);
}

void SocketWithSSL::async_write(char const*  write_buffer, std::size_t size,
                                SocketCompletionCallback on_wrote)
{
    boost::asio::async_write(_stream, boost::asio::buffer(write_buffer, size), on_wrote);
}

errorcode SocketWithSSL::sync_connect(Tcp::endpoint const& ep)
{
    errorcode ec;
    _stream.lowest_layer().connect(ep, ec);
    return ec;
}

errorcode SocketWithSSL::sync_read_some(ReadBuffer& read_buffer, std::size_t& bytes_transferred)
{
    errorcode ec;
    bytes_transferred = _stream.read_some(boost::asio::buffer(read_buffer), ec);
    return ec;
}

errorcode SocketWithSSL::sync_write(const char* write_buffer
                                    , std::size_t write_buffer_size
                                    , std::size_t& bytes_transferred)
{
    errorcode ec;
    bytes_transferred = boost::asio::write(_stream
                                           , boost::asio::buffer(write_buffer, write_buffer_size)
                                           , ec);
    return ec;
}

void SocketWithSSL::close(errorcode& ec)
{
    _stream.lowest_layer().cancel();
    _stream.shutdown(ec);
    _stream.next_layer().close(ec);
}

Tcp::socket& SocketWithSSL::get_socket()
{
    return _stream.next_layer();
}

} // namespace net_detail

namespace
{
std::string port_to_service(unsigned short port)
{
    return std::to_string(port);
}
}

Connection::Connection(IOC& ioc, std::string const& server)
    : _with_ssl(false), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new net_detail::Socket(ioc)),
      _parser(new http_parser), _server(server),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, std::string const& server, std::string const& service)
    : _with_ssl(false), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new net_detail::Socket(ioc)),
      _parser(new http_parser), _server(server), _service(service),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, std::string const& server, unsigned short port)
    : _with_ssl(false), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new net_detail::Socket(ioc)),
      _parser(new http_parser), _server(server), _service(port_to_service(port)),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, boost::asio::ssl::context& ctx, const std::string& server)
    : _with_ssl(true), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new net_detail::SocketWithSSL(ioc, ctx)),
      _parser(new http_parser), _server(server),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
                       , const std::string& service)
    : _with_ssl(true), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new net_detail::SocketWithSSL(ioc, ctx)),
      _parser(new http_parser), _server(server), _service(service),
      _method("GET"), _uri("/"), _http_version("1.1")
{
    init();
}

Connection::Connection(IOC& ioc, boost::asio::ssl::context& ctx, std::string const& server
                       , unsigned short port)
    : _with_ssl(true), _ioc(ioc), _resolver(ioc),
      _socket_ptr(new net_detail::SocketWithSSL(ioc, ctx)),
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
    this->init_parser_setting();
    this->init_parser();

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

void Connection::init_parser_setting()
{
    llhttp_settings_init(&_parser_setting);

    _parser_setting.on_message_begin = &Connection::on_message_begin;
    _parser_setting.on_message_complete = &Connection::on_message_complete;
    _parser_setting.on_headers_complete = &Connection::on_headers_complete;
    _parser_setting.on_header_field = &Connection::on_header_field;
    _parser_setting.on_header_value = &Connection::on_header_value;
    _parser_setting.on_status = &Connection::on_status;
    _parser_setting.on_body = &Connection::on_body;
}

void Connection::init_parser()
{
    llhttp_init(_parser, HTTP_RESPONSE, &_parser_setting);
    _parser->data = this;
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

namespace
{
std::string const default_http_services [] = {"http", "https"};
}

errorcode Connection::do_resolver()
{
    errorcode ec;

    auto results = Utilities::from_host(_server
                                        , (_service.empty()
                                           ? default_http_services[_with_ssl ? 1 : 0] : _service)
                                        , _resolver
                                        , ec);

    if (ec)
    {
        _error = Error::on_resolver;
        _error_msg = "Resolver host address exception.";
        return ec;
    }


#ifdef HAS_RESOLVER_RESULT

    if (results.empty())
    {
        _error = Error::on_resolver;
        _error_msg = "Resolver host address got a empty results.";
    }
    else
    {
        this->_server_endpoint = *results.cbegin();
    }

#else

    if (results == Tcp::resolver::iterator())
    {
        _error = Error::on_resolver;
        _error_msg = "Resolver host address got a empty results.";
    }
    else
    {
        this->_server_endpoint = *results;
    }

#endif

    return ec;
}

void Connection::do_resolver(NotifyFunction notify)
{
    Utilities::from_host(_server
                         , (_service.empty() ? default_http_services[_with_ssl ? 1 : 0] : _service)
                         , _resolver
                         , [this, notify](errorcode const & ec, ResolverResultT results)
    {
        if (ec)
        {
            _error = Error::on_resolver;
            _error_msg = "Resolver host address exception.";
            notify(ec);
            return;
        }

#ifdef HAS_RESOLVER_RESULT

        if (results.empty())
        {
            _error = Error::on_resolver;
            _error_msg = "Resolver host address got a empty results.";
            notify(ec);
            return;
        }

        this->_server_endpoint = *results.cbegin();
#else

        if (results == Tcp::resolver::iterator())
        {
            _error = Error::on_resolver;
            _error_msg = "Resolver host address got a empty results.";
            notify(ec);
            return;
        }

        this->_server_endpoint = *results;
#endif
        this->do_connect(notify);
    });
}

errorcode Connection::do_connect()
{
    if (_is_connected)
    {
        return errorcode();
    }

    auto ec = _socket_ptr->sync_connect(this->_server_endpoint);

    if (!ec)
    {
        _is_connected = true;
        reset();
    }
    else
    {
        _error = Error::on_connect;
        _error_msg = "Connect to server fail. " + ec.message();
    }

    return ec;
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

std::string Connection::make_request_buffer()
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

    return os.str();
}

errorcode Connection::do_write(std::size_t& bytes_transferred)
{
    auto request_buffer = make_request_buffer();

    auto ec = _socket_ptr->sync_write(request_buffer.c_str(), request_buffer.size()
                                      , bytes_transferred);

    if (ec)
    {
        _error = Error::on_write;
        _error_msg = "Write to server fail. " + ec.message();
    }

    return ec;
}

void Connection::do_write(NotifyFunction notify)
{
    auto request_buffer = make_request_buffer();

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

errorcode Connection::do_read(std::size_t& bytes_transferred)
{
    bytes_transferred = 0;

    do
    {
        std::size_t read = 0;
        errorcode ec = _socket_ptr->sync_read_some(_read_buffer, read);
        bytes_transferred += read;

        if (ec)
        {
            _error = Error::on_read;
            _error_msg = "Read from server fail. " + ec.message();
            return ec;
        }

        auto parsed_errno = llhttp_execute(_parser, _read_buffer.data(), read);

        if (parsed_errno != HPE_OK)
        {
            _error = Error::on_read;
            _error_msg = "Parse response content error. " + std::to_string(parsed_errno) + ". "
                         + llhttp_errno_name(parsed_errno) + ".";
            return ec;
        }

    }
    while (_read_complete != read_message_complete);

    return errorcode();
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

        auto parsed_errno = llhttp_execute(_parser, _read_buffer.data(), bytes_transferred);

        if (parsed_errno != HPE_OK)
        {
            _error = Error::on_read;
            _error_msg = "Parse response content error. " + std::to_string(parsed_errno) + ". "
                         + llhttp_errno_name(parsed_errno) + ".";

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

    return HPE_OK;
}

int Connection::on_message_complete(http_parser* parser)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_read_complete = Connection::read_message_complete;

    return HPE_OK;
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

    return HPE_OK;
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

    return HPE_OK;
}

int Connection::on_status(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);
    cnt->_status_buffer.append(at, length);
    return HPE_OK;
}

int Connection::on_body(http_parser* parser, char const* at, size_t length)
{
    Connection* cnt = static_cast<Connection*>(parser->data);

    cnt->_response_body.append(at, length);

    return HPE_OK;
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

bool Connection::ConnectSync()
{
    if (!this->_server_endpoint.port())
    {
        auto ec = this->do_resolver();

        if (ec || HasError())
        {
            return false;
        }
    }

    auto ec = this->do_connect();
    return (!ec && !HasError());
}

void Connection::Write(NotifyFunction notify)
{
    do_write(notify);
}

bool Connection::WriteSync(std::size_t& bytes_transferred)
{
    auto ec = do_write(bytes_transferred);
    return (!ec && !this->HasError());
}

void Connection::Read(NotifyFunction notify)
{
    do_read(notify);
}

bool Connection::ReadSync(std::size_t& bytes_transferred)
{
    auto ec = do_read(bytes_transferred);
    return (!ec && !this->HasError());
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

bool Connection::RequestSync(std::size_t& bytes_wrote, std::size_t& bytes_read
                             , ActionAfterRequest action)
{
    bytes_wrote = 0;
    bytes_read = 0;

    if (!this->_is_connected)
    {
        if (!this->ConnectSync())
        {
            return false;
        }
    }

    auto success = (this->WriteSync(bytes_wrote) && this->ReadSync(bytes_read));

    if (action == ActionAfterRequest::close_connection)
    {
        this->do_close();
    }

    return success;
}

} // namespace Client
} // namespace da4qi4
