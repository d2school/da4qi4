#include "server_engine.hpp"

#include <thread>

#include "def/log_def.hpp"
#include "def/boost_def.hpp"

namespace da4qi4
{

IOContextPool::IOContextPool(std::size_t pool_size)
    : _stopping(false), _next_index(0)
{
    if (pool_size == 0)
    {
        size_t count = std::thread::hardware_concurrency();
        pool_size = (count > 0) ? count : 1;
    }

    for (std::size_t i = 0; i < pool_size; ++i)
    {
        IOContextPtr ioc(new boost::asio::io_context);
        _io_contexts.push_back(ioc);

        _work.push_back(boost::asio::make_work_guard(*ioc));
    }
}

void IOContextPool::Run()
{
    for (std::size_t i = 0; i < _io_contexts.size(); ++i)
    {
        std::shared_ptr<std::thread> thread(new std::thread([i, this]()
        {
            while (!_stopping)
            {
                try
                {
                    errorcode ec;
                    _io_contexts[i]->run(ec);

                    if (ec)
                    {
                        server_logger()->error("Engine running exception. {0}", ec.message());
                    }
                }
                catch (std::exception const& e)
                {
                    server_logger()->error("Engine running exception. {0}", e.what());
                }
                catch (std::string const& s)
                {
                    server_logger()->error("Engine running exception. {0}", s);
                }
                catch (char const* s)
                {
                    server_logger()->error("Engine running exception. {0}", s);
                }
                catch (...)
                {
                    server_logger()->error("Engine running unknown exception.");
                }
            }
        }));

        _threads.push_back(thread);
    }

    for (auto thread_ptr : _threads)
    {
        thread_ptr->join();
    }
}

void IOContextPool::Stop()
{
    if (_stopping)
    {
        return;
    }

    _stopping = true;

    for (auto ioc_ptr : _io_contexts)
    {
        ioc_ptr->stop();
    }
}

boost::asio::io_context& IOContextPool::GetIOContext()
{
    return GetIOContextAndIndex().first;
}

std::pair<boost::asio::io_context&, size_t> IOContextPool::GetIOContextAndIndex()
{
    size_t index = _next_index;
    boost::asio::io_context& io_context = *_io_contexts[index];

    ++_next_index;

    if (_next_index >= _io_contexts.size())
    {
        _next_index = 0;
    }

    return {io_context, index};
}

boost::asio::io_context& IOContextPool::GetIOContextByIndex(size_t index)
{
    assert(index < _io_contexts.size());
    return *(_io_contexts[index]);
}

}
