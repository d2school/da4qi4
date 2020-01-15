#ifndef DAQI_CONNECTION_WEBSOCKET_HPP
#define DAQI_CONNECTION_WEBSOCKET_HPP

#include <memory>
#include <string>
#include <mutex>

#include "daqi/url.hpp"
#include "daqi/net-detail/net_detail_server.hpp"
#include "daqi/websocket/frame.hpp"

namespace da4qi4
{
namespace Websocket
{

class Connection final : public std::enable_shared_from_this<Connection>
{
    Connection(net_detail::SocketInterface* socket, Url&& url, ICHeaders&& headers);

public:
    using Ptr = std::shared_ptr<Connection>;

    static Ptr Create(net_detail::SocketInterface* socket, Url&& url, ICHeaders&& headers);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

public:
    bool IsWithSSL() const
    {
        return (_socket_ptr ? _socket_ptr->IsWithSSL() : false);
    }

    bool HasURLData() const
    {
        return _url != nullptr;
    }

    Url const* GetURLDataPtr() const
    {
        return _url.get();
    }

    void ClearURLData()
    {
        if (_url)
        {
            _url->Clear();
        }
    }

    void ReleaseURLData()
    {
        _url.reset();
    }

    ICHeaders const& GetHeaders() const
    {
        return _headers;
    }

    ICHeaders& GetHeaders()
    {
        return _headers;
    }

    void ClearHeaders()
    {
        _headers.clear();
    }

public:
    void Start();

private:
    void start_handshake();
    void start_write();
    void start_read();

private:
    void append_data_will_write(std::string data);

private:
    void on_read_frame(std::string&& data, FrameType ft, bool is_finished_frame);

private:
    std::unique_ptr<net_detail::SocketInterface> _socket_ptr;
    std::unique_ptr<Url> _url;
    ICHeaders _headers;

private:
    std::mutex _m_4_write;
    std::list<std::string> _data_4_write;

private:
    net_detail::ReadBuffer _buffer_4_read;

private:
    FrameParser _parser;
};

} // namespace Websocket
} // namespace da4qi4

#endif // DAQI_CONNECTION_WEBSOCKET_HPP
