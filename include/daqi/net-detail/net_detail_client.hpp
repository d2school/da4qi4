#ifndef NET_DETAIL_CLIENT_HPP
#define NET_DETAIL_CLIENT_HPP

#include "daqi/def/boost_def.hpp"
#include "daqi/def/asio_def.hpp"

namespace da4qi4
{
namespace Client
{
namespace net_detail
{

using ReadBuffer = std::array<char, 1024 * 2>;
using SocketConnectionCompletionCallback = std::function<void (errorcode const&)>;
using SocketCompletionCallback = std::function<void (errorcode const&, std::size_t)>;

struct SocketBase
{
    virtual ~SocketBase();

    virtual void async_connect(Tcp::endpoint const&, SocketConnectionCompletionCallback) = 0;

    virtual void async_read_some(ReadBuffer&, SocketCompletionCallback) = 0;

    virtual void async_write(char const*, std::size_t, SocketCompletionCallback) = 0;

    virtual errorcode sync_connect(Tcp::endpoint const&) = 0;
    virtual errorcode sync_read_some(ReadBuffer&, std::size_t& bytes_transferred) = 0;
    virtual errorcode sync_write(char const* write_buffer
                                 , std::size_t write_buffer_size
                                 , std::size_t& bytes_transferred) = 0;

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

} // namespace net_detail
} // namespace Client
} // namespace da4qi4


#endif // NET_DETAIL_CLIENT_HPP
