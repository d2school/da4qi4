#ifndef DAQI_CONNECTION_WEBSOCKET_HPP
#define DAQI_CONNECTION_WEBSOCKET_HPP

#include <memory>
#include <string>
#include <mutex>
#include <list>

#include "daqi/url.hpp"
#include "daqi/net-detail/net_detail_server.hpp"

#include "daqi/websocket/frame_websocket.hpp"
#include "daqi/websocket/handler_websocket.hpp"

namespace da4qi4
{
namespace Websocket
{

enum class WriteDataType {continuation = 0, text = 1, binary = 2};

class Connection final : public std::enable_shared_from_this<Connection>
{
    Connection(size_t _ioc_index, net_detail::SocketInterface* socket
               , UrlUnderApp&& url, ICHeaders&& headers, ICCookies&& cookies
               , EventsHandler* handler);

public:
    using Ptr = std::shared_ptr<Connection>;

    static Ptr Create(size_t ioc_index, net_detail::SocketInterface* socket
                      , UrlUnderApp&& url, ICHeaders&& headers, ICCookies&& cookies
                      , EventsHandler* handler);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

public:
    bool IsWithSSL() const
    {
        return (_socket_ptr ? _socket_ptr->IsWithSSL() : false);
    }

    size_t GetIOContextIndex() const
    {
        return _ioc_index;
    }

    Tcp::socket& GetSocket()
    {
        return _socket_ptr->get_socket();
    }

    IOC& GetIOC()
    {
        return _socket_ptr->get_ioc();
    }

public:
    std::string GetURLPath() const;

    std::string const& GetID() const
    {
        return _id;
    }

    void SetID(std::string const& id)
    {
        _id = id;
    }

    Url const* GetURLDataPtr() const
    {
        return _url.get();
    }

    void ClearURLData()
    {
        if (_url)
        {
            makesure_url_path_storaged();
            _url->Clear();
        }
    }

    void ReleaseURLData()
    {
        if (_url)
        {
            makesure_url_path_storaged();
            _url.reset();
        }
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

    ICCookies const& GetCookies() const
    {
        return _cookies;
    }

    ICCookies& GetCookies()
    {
        return _cookies;
    }

    void ClearCookies()
    {
        _cookies.clear();
    }

    void RemoveHttpInfo()
    {
        ReleaseURLData();
        ClearHeaders();
        ClearCookies();
    }

    EventsHandler* GetEventHandler()
    {
        return _evt_handler.get();
    }

    EventsHandler const* GetEventHandler() const
    {
        return _evt_handler.get();
    }

public:
    void Start(Context ctx, std::string const& key);
    void Write(Context ctx, std::string const& data, WriteDataType type, bool finished);
    void Stop(Context ctx);
private:
    void start_handshake(Context ctx, std::string const& key);

    void start_write(Context ctx);
    void start_read(Context ctx);
private:
    void append_write_data(Context ctx, std::string data);

private:
    void on_read_frame(std::string&& data, FrameType ft, bool is_finished_frame);

private:
    void pong(Context ctx);

private:
    void makesure_url_path_storaged()
    {
        if (_full_url_path.empty() && _url)
        {
            _full_url_path = _url->full;
        }
    }
private:
    std::string _id;
    std::string _full_url_path;
    std::unique_ptr<net_detail::SocketInterface> _socket_ptr;
    std::unique_ptr<Url> _url;
    ICHeaders _headers;
    ICCookies _cookies;
private:
    std::mutex _m_4_write;
    std::list<std::string> _data_4_write;

private:
    net_detail::ReadBuffer _buffer_4_read;

private:
    FrameParser _parser;
    Context _ctx_4_parser_callback;
private:
    std::unique_ptr<EventsHandler> _evt_handler;

private:
    std::uint8_t _last_data_type;
    std::uint8_t _error_on_parse;
    std::uint8_t _stop_by_self;

private:
    size_t _ioc_index;
};

class Connections
{
public:
    Connections()
        : _m_ptr(new std::mutex)
    {
    }

    Connections(Connections const&) = delete;
    Connections& operator = (Connections const&) = delete;

    Connections(Connections&& o) = default;

    Connections(Connection::Ptr cnt)
        : _m_ptr(new std::mutex)
    {
        assert(!cnt->GetID().empty());
        _storage.insert(std::make_pair(cnt->GetID(), cnt));
    }

    void Add(Connection::Ptr cnt);

    bool Remove(std::string const& id);
    bool RenameID(std::string const& old_id, std::string const& new_id);

    std::shared_ptr<Connection> Get(std::string const& id);
    std::shared_ptr<Connection> RandOne();

    std::list<Connection::Ptr> All();
    std::list<std::string> AllID();

private:
    std::unique_ptr<std::mutex> _m_ptr;
    std::map<std::string /*id*/, std::weak_ptr<Connection>> _storage;
};

} // namespace Websocket
} // namespace da4qi4

#endif // DAQI_CONNECTION_WEBSOCKET_HPP
