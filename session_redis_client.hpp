#ifndef DAQI_SESSION_REDIS_CLIENT_HPP
#define DAQI_SESSION_REDIS_CLIENT_HPP

#include <map>
#include <queue>
#include <mutex>

#include <boost/asio/deadline_timer.hpp>

#include "def/redis_def.hpp"
#include "def/asio_def.hpp"
#include "def/boost_def.hpp"

#include "session.hpp"

namespace da4qi4
{

class SessionRedisClient
{
    typedef SessionRedisClient Self;
public:
    SessionRedisClient(boost::asio::io_context& ioc
                       , std::string const& host, unsigned short port);

    SessionRedisClient(SessionRedisClient const&) = delete;

    void Connect();

    bool IsConnected() const
    {
        std::lock_guard<std::mutex> guard(_m_connect_status);
        return _client.isConnected();
    }

    void Disconnect()
    {
        std::lock_guard<std::mutex> guard(_m_connect_status);
        _client.disconnect();
    }

    bool IsNeedReconnect() const
    {
        std::lock_guard<std::mutex> guard(_m_connect_status);
        return is_need_connect();
    }

    void Push(SessionData const& data);
    SessionData Pull(std::string const& session_id);

public:
    bool is_need_connect() const
    {
        return (_client.state() == RedisAsyncClient::State::Unconnected)
               || (_client.state() == RedisAsyncClient::State::Closed);
    }

    void do_connect();
    void on_connect_finished(errorcode const& ec);

    void do_write_queue_front_data();

private:
    void OnRedisException(std::string const& error);

private:
    RedisAsyncClient _client;
    boost::asio::deadline_timer _reconect_timer;

    Tcp::endpoint _endpoint;

private:
    mutable std::mutex _m_connect_status;

    mutable std::mutex _m_pulled;
    std::map<std::string, SessionData> _pulled_data;

    mutable std::mutex _m_waitting_push;
    std::queue<SessionData> _waitting_push_data;
};

} //nampespace da4qi4

#endif // DAQI_SESSION_REDIS_CLIENT_HPP
