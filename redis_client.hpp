#ifndef DAQI_REDIS_CLIENT_HPP
#define DAQI_REDIS_CLIENT_HPP

#include <map>
#include <queue>
#include <mutex>
#include <vector>
#include <memory>

#include <boost/asio/deadline_timer.hpp>

#include "def/redis_def.hpp"
#include "def/asio_def.hpp"
#include "def/boost_def.hpp"

#include "session.hpp"

namespace da4qi4
{

class PersistentAsyncRedisClient
{
    typedef PersistentAsyncRedisClient Self;
public:
    PersistentAsyncRedisClient(boost::asio::io_context& ioc
                               , std::string const& host, unsigned short port);

    PersistentAsyncRedisClient(PersistentAsyncRedisClient const&) = delete;

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

    void Command(std::string const& cmd, std::deque<RedisBuffer> args,
                 std::function<void(RedisValue)> handler = redisEmptyHandler);

public:
    bool is_need_connect() const
    {
        return (_client.state() == RedisAsyncClient::State::Unconnected)
               || (_client.state() == RedisAsyncClient::State::Closed);
    }

    void do_connect();

private:
    void OnRedisException(std::string const& error);
    void on_connect_finished(errorcode const& ec);

private:
    RedisAsyncClient _client;
    boost::asio::deadline_timer _reconect_timer;

    Tcp::endpoint _endpoint;

private:
    mutable std::mutex _m_connect_status;
};


class PersistentSyncRedisClient
{
    typedef PersistentSyncRedisClient Self;
public:
    PersistentSyncRedisClient(boost::asio::io_context& ioc
                              , std::string const& host, unsigned short port);

    PersistentSyncRedisClient(PersistentSyncRedisClient const&) = delete;

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

    RedisValue Command(std::string cmd, std::deque<RedisBuffer> args, errorcode& ec);

public:
    bool is_need_connect() const
    {
        return (_client.state() == RedisAsyncClient::State::Unconnected)
               || (_client.state() == RedisAsyncClient::State::Closed);
    }

    void do_connect();
    void on_connect_finished(errorcode const& ec);

private:
    void OnRedisException(std::string const& error);

private:
    RedisSyncClient _client;
    boost::asio::deadline_timer _reconect_timer;

    Tcp::endpoint _endpoint;

private:
    mutable std::mutex _m_connect_status;
};

class PersistentRedisClient
{
public:
    PersistentRedisClient(boost::asio::io_context& ioc
                          , std::string const& host, unsigned short port)
        : _async_client(ioc, host, port)
        , _sync_client(ioc, host, port)
    {

    }

    PersistentAsyncRedisClient const& AsyncClient() const
    {
        return _async_client;
    }

    PersistentAsyncRedisClient& AsyncClient()
    {
        return _async_client;
    }

    PersistentSyncRedisClient const& SyncClient() const
    {
        return _sync_client;
    }

    PersistentSyncRedisClient& SyncClient()
    {
        return _sync_client;
    }

    void Connect();

private:
    PersistentAsyncRedisClient _async_client;
    PersistentSyncRedisClient _sync_client;
};

using RedisClientPtr = PersistentRedisClient*;

class IOContextPool;
class PersistentRedisClientPool
{
    PersistentRedisClientPool() = default;

public:
    void CreateClients(IOContextPool* ioc_pool
                       , std::string const& host, unsigned short port = 6379);

    ~PersistentRedisClientPool()
    {
        clear();
    }

    RedisClientPtr Get(size_t index)
    {
        return (index >= 0 && index < _clients.size()) ? _clients[index] : nullptr;
    }

private:
    void clear();
    std::vector<RedisClientPtr> _clients;

    friend PersistentRedisClientPool& RedisPool();
};

PersistentRedisClientPool& RedisPool();

} //nampespace da4qi4

#endif // DAQI_REDIS_CLIENT_HPP
