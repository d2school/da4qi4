#include "daqi/websocket/connection_websocket.hpp"

#include "daqi/def/log_def.hpp"

#include "daqi/utilities/base64_utilities.hpp"
#include "daqi/utilities/hmac_sha1_utilities.hpp"
#include "daqi/utilities/string_utilities.hpp"

#include "daqi/websocket/context_websocket.hpp"

#include <iostream>

namespace da4qi4
{
namespace Websocket
{

namespace
{

std::string make_handshake_http_response(std::string const& key, std::string const& protocol)
{
    assert(!key.empty());

    std::string data;
    data.reserve(512);

    data.append("HTTP/1.1 101 Switching Protocols\r\n");
    data.append("Upgrade: websocket\r\n");
    data.append("Connection: Upgrade\r\n");
    data.append("Server: da4qi4\r\n");

    std::string accept_key;
    accept_key.reserve(128);
    accept_key.append(key);
    accept_key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    auto digest = Utilities::SHA1(accept_key);

    data.append("Sec-WebSocket-Accept: ");
    data.append(Utilities::Base64Encode(digest.data(), 20));
    data.append("\r\n");

    if (protocol.size())
    {
        data.append("Sec-WebSocket-Protocol: ");
        data.append(protocol);
        data.append("\r\n");
    }

    data.append("\r\n");
    return data;
}

}

Connection::Connection(size_t ioc_index, net_detail::SocketInterface* socket
                       , UrlUnderApp&& url, ICHeaders&& headers
                       , ICCookies&& cookies, EventsHandler* handler)
    : _socket_ptr(socket)
    , _url(new UrlUnderApp(std::move(url))), _headers(std::move(headers)), _cookies(std::move(cookies))
    , _evt_handler(handler)
    , _last_data_type(0), _error_on_parse(0), _stop_by_self(0)
    , _ioc_index(ioc_index)
{
    _parser.RegistMsgCallback(std::bind(&Connection::on_read_frame, this
                                        , std::placeholders::_1
                                        , std::placeholders::_2
                                        , std::placeholders::_3));
}

Connection::Ptr Connection::Create(size_t ioc_index, net_detail::SocketInterface* socket
                                   , UrlUnderApp&& url, ICHeaders&& headers, ICCookies&& cookies
                                   , EventsHandler* handler)
{
    assert(socket != nullptr);
    return Ptr(new Connection(ioc_index, socket, std::move(url), std::move(headers), std::move(cookies), handler));
}

std::string Connection::GetURLPath() const
{
    return (!_full_url_path.empty()) ? _full_url_path : (_url ? _url->full : "");
}

void Connection::Start(Context ctx, std::string const& key)
{
    assert(!key.empty());
    start_handshake(ctx, key);
}

FrameType FromWriteDataType(WriteDataType t)
{
    switch (t)
    {
        case WriteDataType::continuation : return e_continuation;

        case WriteDataType::text : return e_text;

        case WriteDataType::binary : return e_binary;
    }

    return e_continuation;
}

void Connection::Write(Context ctx, std::string const& data, WriteDataType type, bool finished)
{
    auto msg = FrameBuilder().SetFIN(finished).SetFrameType(FromWriteDataType(type)).Build(data);
    append_write_data(ctx, std::move(msg));
}

void Connection::Stop(Context ctx)
{
    _stop_by_self = true;
    auto msg = FrameBuilder().SetFIN(true).SetFrameType(e_connection_close).Build();
    append_write_data(ctx, std::move(msg));
}

void Connection::start_handshake(Context ctx, std::string const& key)
{
    std::string protocol;
    auto it = _headers.find("Sec-WebSocket-Protocol");

    if (it != _headers.end())
    {
        protocol = it->second;
    }

    append_write_data(ctx, make_handshake_http_response(key, protocol));

    //duplex operation for read and write:
    start_read(ctx);
}

void Connection::append_write_data(Context ctx, std::string data)
{
    bool lst_is_empty_bef_this_data;

    {
        std::scoped_lock<std::mutex> lock(_m_4_write);
        lst_is_empty_bef_this_data = _data_4_write.empty();

        _data_4_write.emplace_back(std::move(data));
    }

    if (lst_is_empty_bef_this_data)
    {
        start_write(ctx);
    }
}

void Connection::start_write(Context ctx)
{
    assert(!_data_4_write.empty());

    _socket_ptr->async_write(*_data_4_write.cbegin(), [ctx, this](errorcode const & ec, std::size_t /*wrote_bytes*/)
    {
        if (ec)
        {
            if (ec == boost::asio::error::eof)
            {
                _evt_handler->OnClose(ctx, EventOn::write_event);
            }
            else
            {
                log::Server()->error("Websocket write error. {}. {}.", ec.value(), ec.message());
                _evt_handler->OnError(ctx, EventOn::write_event, ec.value(), ec.message());
            }

            return;
        }

        bool have_data_yet;

        {
            std::scoped_lock<std::mutex> lock(_m_4_write);
            _data_4_write.pop_front();
            have_data_yet = !_data_4_write.empty();
        }

        if (have_data_yet)
        {
            this->start_write(ctx);
        }
    });
}

void Connection::start_read(Context ctx)
{
    _socket_ptr->async_read_some(_buffer_4_read, [ctx, this](errorcode const & ec, std::size_t read_bytes)
    {
        if (ec)
        {
            if (ec == boost::asio::error::eof)
            {
                _evt_handler->OnClose(ctx, EventOn::read_event);
            }
            else
            {
                log::Server()->error("Websocket read error. {}. {}.", ec.value(), ec.message());
                _evt_handler->OnError(ctx, EventOn::read_event, ec.value(), ec.message());
            }

            return;
        }

        assert(read_bytes <= _buffer_4_read.size());

        if (read_bytes)
        {
            _ctx_4_parser_callback = ctx;
            auto [ok, err] = this->_parser.Parse(_buffer_4_read.data(), static_cast<uint32_t>(read_bytes));
            _ctx_4_parser_callback.reset();

            if (!ok)
            {
                log::Server()->error("Parse websocket message fail. {}.", err);
                return;
            }
        }

        if (!_error_on_parse)
        {
            this->start_read(ctx);
        }
    });

    return;
}

void Connection::pong(Context ctx)
{
    auto response_data = FrameBuilder().SetFrameType(e_pong).SetFIN(true).Build();
    this->append_write_data(ctx, std::move(response_data));
}

void Connection::on_read_frame(std::string&& data, FrameType ft, bool is_finished_frame)
{
    _error_on_parse = 0;

    auto ctx = _ctx_4_parser_callback;

    log::Server()->trace("read ws frame-type {}. is-finished {}.", ft, is_finished_frame);

    switch (ft)
    {
        case e_connection_close :
        {
            if (!_stop_by_self)
            {
                this->Stop(ctx);
            }

            return;
        }

        case e_continuation :
        {
            break;
        }

        case e_text :
        {
            _last_data_type = e_text;
            break;
        }

        case e_binary :
        {
            _last_data_type = e_binary;
            break;
        }

        case e_ping :
        {
            this->pong(ctx);
            return;
        }

        default :
        {
            _error_on_parse = 1;
            return;
        }
    }

    switch (_last_data_type)
    {
        case e_text :
            _evt_handler->OnText(ctx, std::move(data), is_finished_frame);
            break;

        case e_binary:
            _evt_handler->OnBinary(ctx, std::move(data), is_finished_frame);
            break;

        default:
            _error_on_parse = 2;
    }
}

void Connections::Add(Connection::Ptr cnt)
{
    assert(_m_ptr != nullptr);
    assert(!cnt->GetID().empty());

    std::scoped_lock<std::mutex> lock(*_m_ptr);
    _storage.insert(std::make_pair(cnt->GetID(), cnt));
}

std::shared_ptr<Connection> Connections::Get(std::string const& id)
{
    assert(_m_ptr != nullptr);

    std::scoped_lock<std::mutex> lock(*_m_ptr);
    auto it = _storage.find(id);

    if (it == _storage.end())
    {
        return nullptr;
    }

    return it->second.lock();
}

std::shared_ptr<Connection> Connections::RandOne()
{
    assert(_m_ptr != nullptr);

    std::scoped_lock<std::mutex> lock(*_m_ptr);
    auto count = _storage.size();

    if (count == 0)
    {
        return nullptr;
    }

    std::map<std::string, std::weak_ptr<Connection>>::iterator it = _storage.begin();
    auto offset = static_cast<std::size_t>(std::rand()) % count;
    std::advance(it, offset);

    return it->second.lock();
}

std::list<Connection::Ptr> Connections::All()
{
    assert(_m_ptr != nullptr);

    std::list<Connection::Ptr> lst;

    {
        std::scoped_lock<std::mutex> lock(*_m_ptr);

        for (auto it : _storage)
        {
            lst.push_back(it.second.lock());
        }
    }

    return lst;
}

std::list<std::string> Connections::AllID()
{
    assert(_m_ptr != nullptr);

    std::list<std::string> lst;

    {
        std::scoped_lock<std::mutex> lock(*_m_ptr);

        for (auto it : _storage)
        {
            lst.push_back(it.first);
        }
    }

    return lst;
}

bool Connections::Remove(std::string const& id)
{
    assert(_m_ptr != nullptr);
    std::scoped_lock<std::mutex> lock(*_m_ptr);

    auto it = _storage.find(id);

    if (it != _storage.end())
    {
        _storage.erase(it);
        return true;
    }

    return false;
}

bool Connections::RenameID(std::string const& old_id, std::string const& new_id)
{
    assert(_m_ptr != nullptr);
    std::scoped_lock<std::mutex> lock(*_m_ptr);

    auto it = _storage.find(old_id);

    if (it != _storage.end())
    {
        std::shared_ptr<Connection> cnt = it->second.lock();
        _storage.erase(it);

        if (!cnt)
        {
            return false;
        }

        if (cnt->GetID() != new_id)
        {
            cnt->SetID(new_id);
        }

        _storage[new_id] = cnt;
        return true;
    }

    return false;
}


} // namespace Websocket
} // namespace da4qi4
