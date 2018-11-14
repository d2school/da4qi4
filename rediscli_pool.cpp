#include "rediscli_pool.hpp"

#include "def/debug_def.hpp"
#include "utilities/asio_utilities.hpp"

#include "server_engine.hpp"

namespace da4qi4
{

void RedisClientPool::CreateClients(IOContextPool* ioc_pool
                                    , const std::string& host, unsigned short port)
{
    for (std::size_t i = 0; i < ioc_pool->Size(); ++i)
    {
        auto client = RedisClientPtr(new RedisClient(ioc_pool->GetIOContextByIndex(i)
                                                     , RedisClientErrorHandlePolicy::auto_reconnect));
        _clients.push_back(client);
        client->Connect(std::bind(&RedisClientPool::on_connect_finished
                                  , this
                                  , std::placeholders::_1), host, port);
    }
}

void RedisClientPool::on_connect_finished(boost::system::error_code const& ec)
{
    if (!ec)
    {
        std::cout << "connect to redis server success." << std::endl;
    }
}

RedisClientPool& RedisPool()
{
    static RedisClientPool _pool;
    return _pool;
}

void RedisClientPool::Stop()
{
    for (auto p : _clients)
    {
        if (p->IsConnected())
        {
            p->Disconnect();
        }
    }
}

void RedisClientPool::clear()
{
    _clients.clear();
}

} //nampespace da4qi4
