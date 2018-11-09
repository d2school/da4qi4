#include "redis_client.hpp"

#include <functional>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "def/debug_def.hpp"
#include "utilities/asio_utilities.hpp"

#include "server_engine.hpp"

namespace da4qi4
{

PersistentAsyncRedisClient::PersistentAsyncRedisClient(boost::asio::io_context& ioc
                                                       , std::string const& host, unsigned short port)
    : _client(ioc)
    , _reconect_timer(ioc)
    , _endpoint(Utilities::make_endpoint(host, port))
{
    _client.installErrorHandler(std::bind(&Self::OnRedisException
                                          , this
                                          , std::placeholders::_1));
}

void PersistentAsyncRedisClient::OnRedisException(std::string const& error)
{
    std::cerr << error << std::endl;

    {
        std::lock_guard<std::mutex> guard(_m_connect_status);

        if (_client.isConnected())
        {
            _client.disconnect();
        }
    }

    do_connect();
}

void PersistentAsyncRedisClient::Connect()
{
    do_connect();
}

void PersistentAsyncRedisClient::do_connect()
{
    std::lock_guard<std::mutex> guard(_m_connect_status);

    if (!is_need_connect())
    {
        return;
    }

    _client.connect(_endpoint, std::bind(&Self::on_connect_finished
                                         , this
                                         , std::placeholders::_1));
}

void PersistentAsyncRedisClient::on_connect_finished(errorcode const& ec)
{
    if (ec)
    {
        std::cerr << ec.message() << std::endl;

        errorcode e;
        _reconect_timer.expires_from_now(boost::posix_time::seconds(1), e);

        if (e)
        {
            std::cerr << "set timer expires for reconnect faile. " << e.message() << std::endl;
            return;
        }

        _reconect_timer.async_wait([this](errorcode const & ec)
        {
            if (ec)
            {
                std::cerr << "timer timer for reconnect faile. " << ec.message() << std::endl;
                return;
            }

            this->do_connect(); //retry
        });

        return;
    }
}

void PersistentAsyncRedisClient::Command(std::string const& cmd, std::deque<RedisBuffer> args,
                                         std::function<void(RedisValue)> handler)
{
    _client.command(std::move(cmd), std::move(args), handler);
}


//////////////////////

PersistentSyncRedisClient::PersistentSyncRedisClient(boost::asio::io_context& ioc
                                                     , std::string const& host, unsigned short port)
    : _client(ioc)
    , _reconect_timer(ioc)
    , _endpoint(Utilities::make_endpoint(host, port))
{
    _client.setConnectTimeout(boost::posix_time::seconds(5));
    _client.setCommandTimeout(boost::posix_time::seconds(5));

    _client.installErrorHandler(std::bind(&Self::OnRedisException
                                          , this
                                          , std::placeholders::_1));
}

void PersistentSyncRedisClient::OnRedisException(std::string const& error)
{
    std::cerr << error << std::endl;

    {
        std::lock_guard<std::mutex> guard(_m_connect_status);

        if (_client.isConnected())
        {
            _client.disconnect();
        }
    }

    do_connect();
}

void PersistentSyncRedisClient::Connect()
{
    do_connect();
}

void PersistentSyncRedisClient::do_connect()
{
    std::lock_guard<std::mutex> guard(_m_connect_status);

    if (!is_need_connect())
    {
        return;
    }

    errorcode ec;
    _client.connect(_endpoint, ec);
    on_connect_finished(ec);
}

void PersistentSyncRedisClient::on_connect_finished(errorcode const& ec)
{
    if (ec)
    {
        std::cerr << ec.message() << std::endl;

        errorcode e;
        _reconect_timer.expires_from_now(boost::posix_time::seconds(1), e);

        if (e)
        {
            std::cerr << "set timer expires for reconnect faile. " << e.message() << std::endl;
            return;
        }

        _reconect_timer.async_wait([this](errorcode const & ec)
        {
            if (ec)
            {
                std::cerr << "timer timer for reconnect faile. " << ec.message() << std::endl;
                return;
            }

            this->do_connect(); //retry
        });

        return;
    }
}


RedisValue PersistentSyncRedisClient::Command(std::string cmd, std::deque<RedisBuffer> args, errorcode& ec)
{
    return _client.command(std::move(cmd), std::move(args), ec);
}

/////

void PersistentRedisClient::Connect()
{
    try
    {
        _async_client.Connect();
        _sync_client.Connect();
    }
    catch (std::exception const& e)
    {
        std::cerr << "redis (async or sync) client connect exception." << std::endl;
    }
}

/////

void PersistentRedisClientPool::CreateClients(IOContextPool* ioc_pool
                                              , const std::string& host, unsigned short port)
{
    for (std::size_t i = 0; i < ioc_pool->Size(); ++i)
    {
        boost::asio::io_context& ioc = ioc_pool->GetIOContextByIndex(i);

        auto client = new PersistentRedisClient(ioc, host, port);
        client->Connect();
        _clients.push_back(client);
    }
}

PersistentRedisClientPool& RedisPool()
{
    static PersistentRedisClientPool _pool;
    return _pool;
}

void PersistentRedisClientPool::clear()
{
    for (auto p : _clients)
    {
        delete p;
    }

    _clients.clear();
}

} //nampespace da4qi4
