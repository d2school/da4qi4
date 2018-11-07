#include "session_redis_client.hpp"

#include <functional>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "def/debug_def.hpp"
#include "utilities/asio_utilities.hpp"

namespace da4qi4
{

SessionRedisClient::SessionRedisClient(boost::asio::io_context& ioc
                                       , std::string const& host, unsigned short port)
    : _client(ioc)
    , _reconect_timer(ioc)
    , _endpoint(Utilities::make_endpoint(host, port))
{
    _client.installErrorHandler(std::bind(&Self::OnRedisException
                                          , this
                                          , std::placeholders::_1));
}

void SessionRedisClient::OnRedisException(std::string const& error)
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

void SessionRedisClient::do_connect()
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

void SessionRedisClient::on_connect_finished(errorcode const& ec)
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

void SessionRedisClient::do_write_queue_front_data()
{
    std::lock_guard<std::mutex> guard(_m_waitting_push);

    if (_waitting_push_data.empty())
    {
        return;
    }

    auto const& front = _waitting_push_data.front();

    std::string sid = front.GetID();
    std::string max_age = std::to_string(front.cookie.GetMaxAge());
    std::string body = front.ToString();

    _client.command("SETEX", {sid, body, max_age}, [this](RedisValue value)
    {
        if (value.isError())
        {
            std::cerr << value.toString() << std::endl;
            return;
        }

        _waitting_push_data.pop();
        do_write_queue_front_data();
    });
}

void SessionRedisClient::Push(SessionData const& data)
{
    std::lock_guard<std::mutex> guard(_m_waitting_push);
    bool is_empty_bef = _waitting_push_data.empty();

    _waitting_push_data.push(data);

    if (is_empty_bef)
    {
        do_write_queue_front_data();
    }
}


} //nampespace da4qi4
