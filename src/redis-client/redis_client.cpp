#include "daqi/redis-client/redis_client.hpp"

#include <iostream>
#include <functional>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "daqi/def/log_def.hpp"

#include "daqi/redis-client/redis_command.hpp"
#include "daqi/redis-client/redis_parser.hpp"


namespace da4qi4
{

namespace
{

unsigned int redis_server_reconnect_interval_seconds(unsigned int const reconnect_count)
{
    if (reconnect_count < 10) //0s~10s
    {
        return 1;
    }

    if (reconnect_count < 32) //10s~2m
    {
        return 5;
    }

    //after 2 minutes
    if (reconnect_count < 200) //2m~0.5h
    {
        return 10;
    }

    if (reconnect_count < 260)  //0.5h~1h
    {
        return 30;
    }

    // 1h
    return 300; //reconnect 1 time per 5 minutes
}

RedisValue bad_redis_value()
{
    return RedisValue("bad redis-value. parse fail", RedisValue::ErrorTag());
}

RedisValue bad_redis_value(std::string const& content)
{
    return RedisValue("bad redis-value. parse fail [" + content + "]"
                      , RedisValue::ErrorTag());
}

} //namespace

void RedisClient::Disconnect()
{
    if (IsConnected())
    {
        do_disconnect();
    }
    else
    {
        boost::system::error_code ec;
        _reconnect_timer.cancel(ec);
    }
}

void RedisClient::do_disconnect()
{
    boost::system::error_code ec;
    _socket.cancel(ec);
    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    _socket.close(ec);
    _connect_status = not_connect;
}

bool RedisSyncClient::do_connect()
{
    _connect_status = is_connectting;

    boost::system::error_code ec;

    boost::asio::ip::tcp::endpoint end_point(boost::asio::ip::address::from_string(_host), _port);

    _socket.connect(end_point, ec);

    if (ec)
    {
        _connect_status = not_connect;
        return false;
    }

    _connect_status = is_connected;
    return true;
}

RedisValue RedisSyncClient::Command(std::string cmd, std::deque<RedisBuffer> args)
{
    assert(!cmd.empty());

    args.emplace_front(std::move(cmd));
    auto send_data = MakeCommand(args);

    boost::system::error_code ec;
    boost::asio::write(_socket, boost::asio::buffer(send_data), ec);

    if (ec)
    {
        return RedisValue(ec.message(), RedisValue::ErrorTag());
    }

    RedisParser parser;

    std::size_t beg = 0;
    bool completed = false;

    std::string read_data;
    read_data.reserve(_read_buffer_size_ << 1);

    while (!completed)
    {
        char tmp_buf[_read_buffer_size_];
        std::size_t read_size = _socket.read_some(boost::asio::buffer(tmp_buf, _read_buffer_size_), ec);

        if (ec)
        {
            return RedisValue(ec.message(), RedisValue::ErrorTag());
        }

        if (read_size > 0)
        {
            read_data.append(tmp_buf, read_size);
            size_t end = read_data.size();

            while (!completed && (beg < end))
            {
                auto result = parser.Parse(read_data.c_str() + beg, end - beg);

                switch (result.second)
                {
                    case RedisParser::Completed:
                        completed = true;
                        break;

                    case RedisParser::Incompleted:
                        beg += result.first;
                        break;

                    default :
                        return bad_redis_value();
                }
            }
        }
    }

    return (completed) ? parser.Result() : bad_redis_value();
}

bool RedisSyncClient::Connect(std::string const& host, unsigned short port)
{
    if (IsConnected() || IsConnectting())
    {
        return true;
    }

    _host = host;
    _port = port;

    return do_connect();
}

RedisClient::~RedisClient()
{
    Disconnect();
}

void RedisClient::Connect(std::function<void (boost::system::error_code const& ec)> on,
                          std::string const& host, unsigned short port)
{
    if (IsConnected() || IsConnectting())
    {
        return;
    }

    _host = host;
    _port = port;

    do_async_connect(on);
}

void RedisClient::Reconnect(std::function<void (boost::system::error_code const& ec)> on)
{
    if (IsConnectting())
    {
        if (on)
        {
            auto ec = boost::system::errc::make_error_code(boost::system::errc::operation_in_progress);
            on(ec);
        }

        return;
    }

    if (!IsConnected())
    {
        boost::system::error_code ec;
        _reconnect_timer.cancel(ec);
        start_reconnect_timer(on);
    }
    else if (on)
    {
        auto ec = boost::system::errc::make_error_code(boost::system::errc::already_connected);
        on(ec);
    }
}

void RedisClient::do_async_connect(std::function<void (boost::system::error_code const& ec)> on)
{
    _connect_status = is_connectting;
    boost::asio::ip::tcp::endpoint end_point(
                boost::asio::ip::address::from_string(_host), _port);

    auto cb = std::bind(&RedisClient::on_connect_finished, this, on, std::placeholders::_1);
    _socket.async_connect(end_point, cb);
}

enum class AsyncHandlerAction
{
    on_async_connect, on_async_read, on_async_write, on_reply_parse
};

char const* get_async_handler_action_name(AsyncHandlerAction aha)
{
    static char const* names[] = {"connect", "read", "write", "parse"};

    switch (aha)
    {
        case AsyncHandlerAction::on_async_connect:
            return names[0];

        case AsyncHandlerAction::on_async_read:
            return names[1];

        case AsyncHandlerAction::on_async_write:
            return names[2];

        case AsyncHandlerAction::on_reply_parse:
            return names[3];
    }

    return "";
}

void try_on_redis_handler(std::function<void (boost::system::error_code const& ec)> on
                          , boost::system::error_code p, AsyncHandlerAction aha)
{
    if (!on)
    {
        return;
    }

    try
    {
        on(std::move(p));
    }
    catch (std::exception const& e)
    {
        log::Server()->error("Redis handle {} result exception. {}", get_async_handler_action_name(aha), e.what());
    }
    catch (std::string const& s)
    {
        log::Server()->error("Redis handle {} result exception. {}", get_async_handler_action_name(aha), s);
    }
    catch (char const* s)
    {
        log::Server()->error("Redis handle {} result exception. {}", get_async_handler_action_name(aha), s);
    }
    catch (...)
    {
        log::Server()->error("Redis handle {} result unknown exception.", get_async_handler_action_name(aha));
    }
}

void try_on_redis_handler(std::function<void(RedisValue value)> on, RedisValue p, AsyncHandlerAction aha)
{
    if (!on)
    {
        return;
    }

    //debug:
    RedisValue debug_dump;
    if (aha == AsyncHandlerAction::on_reply_parse)
    {
        debug_dump = p;
    }

    try
    {
        on(std::move(p));
    }
    catch (std::exception const& e)
    {
        log::Server()->error("Redis handle {} result exception. {}", get_async_handler_action_name(aha), e.what());

        if (!debug_dump.IsNull())
        {
           log::Server()->debug("context read from redis: {}.", debug_dump.ToString());
        }
    }
    catch (std::string const& s)
    {
        log::Server()->error("Redis handle {} result exception. {}", get_async_handler_action_name(aha), s);
    }
    catch (char const* s)
    {
        log::Server()->error("Redis handle {} result exception. {}", get_async_handler_action_name(aha), s);
    }
    catch (...)
    {
        log::Server()->error("Redis handle {} result unknown exception.", get_async_handler_action_name(aha));
    }
}

void RedisClient::on_connect_finished(std::function<void (boost::system::error_code const& ec)> on
                                      , boost::system::error_code const& ec)
{
    _connect_status = (!ec) ? is_connected : not_connect;

    if (ec)
    {
        _connect_status = not_connect;
        log::Server()->error("Connect to redis server fail. {}", ec.message());
    }
    else
    {
        if (_reconnect_count > 0)
        {
            log::Server()->info("Reconnect to redis server success, after {} time(s).", _reconnect_count);
            _reconnect_count = 0;
        }

        _connect_status = is_connected;
    }

    try_on_redis_handler(on, std::move(ec), AsyncHandlerAction::on_async_connect);

    if (ec && (_error_handle_policy == RedisClientErrorHandlePolicy::auto_reconnect))
    {
        start_reconnect_timer(on);
    }
}

bool RedisClient::start_reconnect_timer(std::function<void (boost::system::error_code const& ec)> on)
{
    auto seconds = redis_server_reconnect_interval_seconds(++_reconnect_count);

    boost::system::error_code timer_ec;
    _reconnect_timer.expires_from_now(boost::posix_time::seconds(seconds), timer_ec);

    if (timer_ec)
    {
        log::Server()->error("Set reconnect redis server timer expires time fail. {}", timer_ec.message());
        return false;
    }

    _reconnect_timer.async_wait([this, on](boost::system::error_code const & ec)
    {
        if (ec)
        {
            log::Server()->error("Start reconnect redis server timer fail. {}", ec.message());
            return;
        }

        do_async_connect(on);
    });

    return true;
}


RedisValue RedisClient::Command(std::string cmd, std::deque<RedisBuffer> args, asio_yield_ctx ytx)
{
    assert(!cmd.empty());

    args.emplace_front(std::move(cmd));
    auto command_data = MakeCommand(args);

    try
    {
        return do_yield_command(std::move(command_data), ytx);
    }
    catch (std::exception const& ex)
    {
        return RedisValue(std::string("yield redis command exception. ") + ex.what(), RedisValue::ErrorTag());
    }
}

RedisValue RedisClient::do_yield_command(std::vector<char> command_data, boost::asio::yield_context ytx)
{
    boost::system::error_code ec;

    boost::asio::async_write(_socket, boost::asio::buffer(command_data), ytx[ec]);

    if (ec)
    {
        RedisValue error_value(ec.message(), RedisValue::ErrorTag());

        if (_error_handle_policy == RedisClientErrorHandlePolicy::auto_reconnect)
        {
            Reconnect();
        }

        return error_value;
    }

    std::string reply_buf;
    reply_buf.reserve(1024 + 512);
    std::size_t reply_parse_beg = 0;

    bool io_error = false;
    RedisParser parser;
    RedisValue value;

    for(;;)
    {
        size_t const  tmp_read_buffer = 1024;
        char tmp_read_buf[tmp_read_buffer];

        std::size_t bytes = _socket.async_read_some(boost::asio::buffer(tmp_read_buf, tmp_read_buffer), ytx[ec]);

        if (ec)
        {
            io_error = true;
            value = RedisValue(ec.message(), RedisValue::ErrorTag());
            break;
        }

        if (bytes == 0)
        {
            continue;
        }

        reply_buf.append(tmp_read_buf, bytes);

        size_t end = reply_buf.size();

        bool completed = false, error = false;

        while (!completed && !error && (reply_parse_beg < end))
        {
            auto result = parser.Parse(reply_buf.c_str() + reply_parse_beg, end - reply_parse_beg);
            reply_parse_beg += result.first;

            switch (result.second)
            {
                case RedisParser::Completed:
                    completed = true;
                    break;

                case RedisParser::Incompleted:
                    break;

                default :
                    error = true;
                    break;
            }
        }

        if (completed)
        {
            value = parser.Result();
            break;
        }

        if (error)
        {
            value = bad_redis_value(reply_buf);
            break;
        }
    }

    if (io_error && (_error_handle_policy == RedisClientErrorHandlePolicy::auto_reconnect))
    {
        Reconnect();
    }

    return value;
}


} // namespace da4qi4

