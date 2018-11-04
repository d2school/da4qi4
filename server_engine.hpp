#ifndef DAQI_SERVER_ENGINE_HPP
#define DAQI_SERVER_ENGINE_HPP

#include <memory>

#include <list>
#include <vector>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace da4qi4
{

class IOContextPool
    : private boost::noncopyable
{
public:
    explicit IOContextPool(std::size_t pool_size = 0);

    void Run();
    void Stop();

    size_t Size() const
    {
        return _ioc_for_connections.size();
    }

    boost::asio::io_context& GetServerIOContext()
    {
        return _ioc_for_server;
    }

    boost::asio::io_context& GetConnectionIOContext();

private:
    using IOContextPtr = std::shared_ptr<boost::asio::io_context>;
    using IOContextWork = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    std::atomic_bool _stopping;

    boost::asio::io_context _ioc_for_server;
    std::vector<IOContextPtr> _ioc_for_connections;

    std::list<IOContextWork> _work;
    std::size_t _next_index;

    std::vector<std::shared_ptr<std::thread>> _threads;
};

} //namespace da4qi4
#endif // DAQI_SERVER_ENGINE_HPP
