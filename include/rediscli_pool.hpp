#ifndef DAQI_REDIS_CLI_POOL_HPP
#define DAQI_REDIS_CLI_POOL_HPP

#include <map>
#include <queue>
#include <mutex>
#include <vector>
#include <memory>
#include <functional>

#include <boost/asio/deadline_timer.hpp>

#include "def/redis_def.hpp"
#include "redis-client/redis_client.hpp"

#include "session.hpp"

namespace da4qi4
{

using RedisClientPtr = std::shared_ptr<RedisClient>;

class IOContextPool;
class RedisClientPool
{
    RedisClientPool() = default;

public:
    void CreateClients(IOContextPool* ioc_pool
                       , std::string const& host = redis_server_default_host
                       , unsigned short port = redis_server_default_port);

    void Stop();

    ~RedisClientPool()
    {
        clear();
    }

    RedisClientPtr Get(size_t index)
    {
        return (index >= 0 && index < _clients.size()) ? _clients[index] : nullptr;
    }

private:
    void clear();
    void on_connect_finished(std::size_t index, boost::system::error_code const& ec);
private:
    std::vector<RedisClientPtr> _clients;

    friend RedisClientPool& RedisPool();
};

RedisClientPool& RedisPool();

} //nampespace da4qi4

#endif // DAQI_REDIS_CLI_POOL_HPP
