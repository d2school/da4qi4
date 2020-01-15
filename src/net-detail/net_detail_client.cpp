#include "daqi/net-detail/net_detail_client.hpp"

namespace da4qi4
{
namespace Client
{

namespace net_detail
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
    _stream.lowest_layer().async_connect(ep
                                         , [this, on_connect](errorcode const & ec)
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
                                           , boost::asio::buffer(write_buffer, write_buffer_size), ec);
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
} // namespace Client
} // namespace da4qi4
