#ifndef DAQI_SERVER_ENGINE_HPP
#define DAQI_SERVER_ENGINE_HPP

#include <memory>
#include <thread>

#include <list>
#include <vector>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include "def/asio_def.hpp"

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
        return _io_contexts.size();
    }

    IOC& GetIOContext();

    std::pair<IOC&, size_t> GetIOContextAndIndex();
    IOC& GetIOContextByIndex(size_t index);

private:
    using IOContextPtr = std::shared_ptr<IOC>;
#ifdef HAS_IO_CONTEXT
    using IOContextWork = boost::asio::executor_work_guard<IOC::executor_type>;
#else
    using IOContextWork = std::unique_ptr<IOC::work>;
#endif

    std::atomic_bool _stopping;

    std::vector<IOContextPtr> _io_contexts;

    std::list<IOContextWork> _work;
    std::size_t _next_index;

    std::vector<std::shared_ptr<std::thread>> _threads;
};

} //namespace da4qi4
#endif // DAQI_SERVER_ENGINE_HPP
