#include "server_engine.hpp"

#include <thread>

#include "def/debug_def.hpp"
#include "def/boost_def.hpp"

namespace da4qi4
{

IOContextPool::IOContextPool(std::size_t pool_size)
    : _stopping(false), _next_index(0)
{
    if (pool_size == 0)
    {
        size_t count = std::thread::hardware_concurrency();
        pool_size = (count > 2) ? (count - 1) : 1;
    }

    for (std::size_t i = 0; i < pool_size; ++i)
    {
        IOContextPtr ioc(new boost::asio::io_context);
        _ioc_for_connections.push_back(ioc);

        _work.push_back(boost::asio::make_work_guard(*ioc));
    }
}

void IOContextPool::Run()
{
    for (std::size_t i = 0; i < _ioc_for_connections.size(); ++i)
    {
        std::shared_ptr<std::thread> thread(new std::thread([i, this]()
        {
            while (!_stopping)
            {
                try
                {
                    errorcode ec;
                    _ioc_for_connections[i]->run(ec);

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

    while (!_stopping)
    {
        try
        {
            errorcode e;
            _ioc_for_server.run(e);

            if (e)
            {
                std::cerr << e.message() << std::endl;
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

    for (auto thread_ptr : _threads)
    {
        thread_ptr->join();
    }
}

void IOContextPool::Stop()
{
    _stopping = true;

    _ioc_for_server.stop();

    for (auto ioc_ptr : _ioc_for_connections)
    {
        ioc_ptr->stop();
    }
}

boost::asio::io_context& IOContextPool::GetConnectionIOContext()
{
    boost::asio::io_context& io_context = *_ioc_for_connections[_next_index];

    ++_next_index;

    if (_next_index >= _ioc_for_connections.size())
    {
        _next_index = 0;
    }

    return io_context;
}
}
