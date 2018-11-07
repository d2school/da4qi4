#include "session_redis.hpp"

#include <boost/bind.hpp>

#include "def/debug_def.hpp"
#include "utilities/asio_utilities.hpp"


namespace da4qi4
{
/*


void RedisSession::do_connect()
{
    {
        std::lock_guard<std::mutex> lock(_m);

        if (_connect_status != no_connect)
        {
            return;
        }

        _connect_status = is_connectting;
    }

    _redisclient.connect(_endpoint, boost::bind(&RedisSession::on_connect_finished
                                                , this
                                                , boost::asio::placeholders::error));
}

void RedisSession::on_connect_finished(errorcode const& ec)
{
    if (ec)
    {
        std::cerr << ec.message() << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(_m);
    _connect_status = is_connected;

    for (auto it = _nodes.begin(); ++)
    }

void RedisSession::Push(std::string session_id, RedisSessionData data)
{
    bool reconnect_now = false;
    int max_age = 0;

    std::string json_string_will_write_to_redis;

    {
        std::lock_guard<std::mutex> lock(_m);

        if (_connect_status == is_connected)
        {
            write_now = true;
            json_string_will_write_to_redis = data.ToJSonString();
            max_age = data.cookie.GetMaxAge();
        }
        else if (_connect_status == no_connect)
        {
            reconnect_now = true;
        }

        RedisSessionNode node(std::move(data));
        _nodes[session_id] = std::move(node);
    }

    if (!write_now)
    {
        if (reconnect_now)
        {
            do_connect();
        }

        return;
    }

    assert(!json_string_will_write_to_redis.empty());
    async_write_data(std::move(session_id), json_string_will_write_to_redis, max_age);
}

void RedisSession::async_write_data(std::string session_id, std::string data, int max_age)
{
    if (max_age < 0)
    {
        max_age = 3600;
    }

    std::string max_age_str = std::to_string(max_age);
    _redisclient.command("SETEX"
                         , {session_id, std::move(data), std::move(max_age_str)}
                         , [session_id, this](RedisValue value)
    {
        if (value.isOk())
        {
            std::lock_guard<std::mutex> lock(_m);
            auto it = _nodes.find(session_id);

            if (it != _nodes.end() && !it->second.posted)
            {
                it->second.posted = true;
            }
        }
    });
}

void RedisSession::Pull(std::string const& session_id)
{

}

*/

} //namespace da4qi4
