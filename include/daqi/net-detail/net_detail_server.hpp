#ifndef DAQI_NET_DETAIL_SERVER_HPP
#define DAQI_NET_DETAIL_SERVER_HPP

#include <boost/asio/ssl/stream.hpp>

#include "daqi/def/asio_def.hpp"
#include "daqi/def/boost_def.hpp"

namespace da4qi4
{
namespace net_detail
{

using ReadBuffer = std::array<char, 1024 * 4>;
using WriteBuffer = boost::asio::streambuf;
using ChunkedBuffer = std::string;

using SocketCompletionCallback = std::function<void (errorcode const&, std::size_t)>;

struct SocketInterface
{
    virtual ~SocketInterface();

    virtual void close(errorcode& ec) = 0;

    virtual IOC& get_ioc() = 0;
    virtual Tcp::socket& get_socket() = 0;

    virtual void async_read_some(ReadBuffer&, SocketCompletionCallback) = 0;
    virtual void async_write(WriteBuffer&, SocketCompletionCallback) = 0;
    virtual void async_write(ChunkedBuffer const&, SocketCompletionCallback) = 0;

    virtual bool IsWithSSL() const = 0;
};

struct Socket : SocketInterface
{
    Socket(IOC& ioc)
        : _socket(ioc)
    {
    }

    ~Socket() override;

    void close(errorcode& ec) override
    {
        _socket.shutdown(boost::asio::socket_base::shutdown_both, ec);
        _socket.close(ec);
    }

    bool IsWithSSL() const override
    {
        return false;
    }

    IOC& get_ioc() override;

    Tcp::socket& get_socket() override
    {
        return _socket;
    }

    void async_read_some(ReadBuffer& read_buffer, SocketCompletionCallback on_read) override
    {
        _socket.async_read_some(boost::asio::buffer(read_buffer), on_read);
    }

    void async_write(WriteBuffer& write_buffer, SocketCompletionCallback on_wrote) override
    {
        boost::asio::async_write(_socket, write_buffer, on_wrote);
    }

    void async_write(ChunkedBuffer const& chunked_buffer, SocketCompletionCallback on_wrote) override
    {
        boost::asio::async_write(_socket, boost::asio::buffer(chunked_buffer), on_wrote);
    }

private:
    Tcp::socket _socket;
};

struct SocketWithSSL : SocketInterface
{
    SocketWithSSL(IOC& ioc, boost::asio::ssl::context& ssl_ctx)
        : _stream(ioc, ssl_ctx)
    {
    }

    ~SocketWithSSL() override;

    void close(errorcode& ec) override
    {
        _stream.lowest_layer().cancel();
        _stream.shutdown(ec);
        _stream.next_layer().close(ec);
    }

    bool IsWithSSL() const override
    {
        return true;
    }

    IOC& get_ioc() override;

    Tcp::socket& get_socket() override
    {
        return _stream.next_layer();
    }

    void async_read_some(ReadBuffer& read_buffer, SocketCompletionCallback on_read) override
    {
        _stream.async_read_some(boost::asio::buffer(read_buffer), on_read);
    }

    void async_write(WriteBuffer& write_buffer, SocketCompletionCallback on_wrote) override
    {
        boost::asio::async_write(_stream, write_buffer, on_wrote);
    }

    void async_write(ChunkedBuffer const& chunked_buffer, SocketCompletionCallback on_wrote) override
    {
        boost::asio::async_write(_stream, boost::asio::buffer(chunked_buffer), on_wrote);
    }

public:
    boost::asio::ssl::stream<Tcp::socket>& get_stream()
    {
        return _stream;
    }

private:
    boost::asio::ssl::stream<Tcp::socket> _stream;
};

} //namespace net_detail
} // namespace da4qi4

#endif // DAQI_NET_DETAIL_SERVER_HPP
