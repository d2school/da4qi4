#include "engine.hpp"

#include <thread>

#include "def/debug_def.hpp"
#include "def/boost_def.hpp"

namespace da4qi4
{

IOContextPool::IOContextPool(std::size_t pool_size)
    : _next_index(0)
{
    if (pool_size == 0)
    {
        size_t count = std::thread::hardware_concurrency();
        pool_size = (count > 2) ? (count - 1) : 1;
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
                        std::cerr << ec.message() << std::endl;
                    }
                }
                catch (std::exception const& e)
                {
                    std::cerr << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "unknown exception." << std::endl;
                }
            }
        }));

        _threads.push_back(thread);
    }
}

void IOContextPool::Wait()
{
    for (auto thread_ptr : _threads)
    {
        thread_ptr->join();
    }
}

void IOContextPool::Stop()
{
    _stopping = true;

    for (auto ioc_ptr : _io_contexts)
    {
        ioc_ptr->stop();
    }
}

boost::asio::io_context& IOContextPool::GetIOContext()
{
    boost::asio::io_context& io_context = *_io_contexts[_next_index];

    ++_next_index;

    if (_next_index >= _io_contexts.size())
    {
        _next_index = 0;
    }

    return io_context;
}
}
