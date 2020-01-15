#include "daqi/websocket/connection_websocket.hpp"

#include "daqi/def/log_def.hpp"

#include "daqi/utilities/base64_utilities.hpp"
#include "daqi/utilities/hmac_sha1_utilities.hpp"
#include "daqi/utilities/string_utilities.hpp"

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

Connection::Connection(net_detail::SocketInterface* socket, Url&& url, ICHeaders&& headers)
    : _socket_ptr(socket), _url(new Url(std::move(url))), _headers(std::move(headers))
{
    _parser.RegistMsgCallback(std::bind(&Connection::on_read_frame, this
                                        , std::placeholders::_1
                                        , std::placeholders::_2
                                        , std::placeholders::_3));
}

Connection::Ptr Connection::Create(net_detail::SocketInterface* socket, Url&& url, ICHeaders&& headers)
{
    assert(socket != nullptr);
    return Ptr(new Connection(socket, std::move(url), std::move(headers)));
}

void Connection::Start()
{
    start_handshake();
}

void Connection::start_handshake()
{
    auto upgrade_it = _headers.find("Upgrade");

    if (upgrade_it == _headers.end())
    {
        return;
    }

    auto key_it = _headers.find("Sec-WebSocket-Key");

    if (key_it == _headers.end())
    {
        return;
    }

    if (!Utilities::iEquals(upgrade_it->second, "websocket") || key_it->second.empty())
    {
        return;
    }

    std::string protocol;
    auto protocol_it = _headers.find("Sec-WebSocket-Protocol");

    if (protocol_it != _headers.end())
    {
        protocol = protocol_it->second;
    }

    append_data_will_write(make_handshake_http_response(key_it->second, protocol));
}

void Connection::append_data_will_write(std::string data)
{
    bool lst_is_empty_bef_this_data;

    {
        std::scoped_lock<std::mutex> lock(_m_4_write);
        lst_is_empty_bef_this_data = _data_4_write.empty();

        _data_4_write.emplace_back(std::move(data));
    }

    if (lst_is_empty_bef_this_data)
    {
        start_write();
    }
}

void Connection::start_write()
{
    assert(!_data_4_write.empty());

    auto self = this->shared_from_this();

    _socket_ptr->async_write(*_data_4_write.cbegin(), [self, this](errorcode const & ec, std::size_t wrote_bytes)
    {
        if (ec)
        {
            //todo : on_close();
            log::Server()->debug("wrote error. {}. {}.", ec.value(), ec.message());
            return;
        }

        log::Server()->debug("wrote {} bytes.", wrote_bytes);

        bool have_data_yet;

        {
            std::scoped_lock<std::mutex> lock(_m_4_write);
            _data_4_write.pop_front();
            have_data_yet = !_data_4_write.empty();
        }

        if (have_data_yet)
        {
            this->start_write();
        }
        else
        {
            this->start_read();
        }
    });
}

void Connection::start_read()
{
    auto self = this->shared_from_this();

    _socket_ptr->async_read_some(_buffer_4_read, [self, this](errorcode const & ec, std::size_t read_bytes)
    {
        if (ec)
        {
            //todo : on_close();
            log::Server()->debug("read error. {}. {}.", ec.value(), ec.message());
            return;
        }

        log::Server()->debug("read {} bytes.", read_bytes);

        if (read_bytes == 0)
        {
            this->start_read();
            return;
        }

        assert(read_bytes <= _buffer_4_read.size());

        auto [ok, err] = this->_parser.Parse(_buffer_4_read.data(), static_cast<uint32_t>(read_bytes));

        if (!ok)
        {
            log::Server()->error("Parse websocket message fail. {}.", err);
            return;
        }
    });

    return;
}

void Connection::on_read_frame(std::string&& data, FrameType ft, bool is_finished_frame)
{
    log::Server()->trace("read ws frame-type {}. is-finished {}.", ft, is_finished_frame);

    FrameBuilder builder;

    switch (ft)
    {
        case e_connection_close :
        {
            //todo : on_close();
            auto close_response_data = builder.SetFrameType(e_connection_close).SetFIN(true).Build();
            this->append_data_will_write(std::move(close_response_data));
            return;
        }

        case e_continuation :
        {
            break;
        }

        case e_text :
        {
            log::Server()->trace(data);
            break;
        }

        case e_binary :
        {
            break;
        }

        case e_ping :
        {
            auto response_data = builder.SetFrameType(e_pong).SetFIN(true).Build();
            this->append_data_will_write(std::move(response_data));
            return;
        }

        default :
            return;
    }

    std::string tm = std::to_string(std::time(nullptr));
    auto response_data = builder.SetFrameType(e_text).SetFIN(true).Build(("Hello WebSocket! " + tm));
    this->append_data_will_write(std::move(response_data));
    return;
}

} // namespace Websocket
} // namespace da4qi4
