#ifndef DAQI_REDIS_CLIENT_HPP
#define DAQI_FREDIS_CLIENT_HPP

#include <string>
#include <queue>
#include <deque>

#include <boost/noncopyable.hpp>

#include "daqi/def/asio_def.hpp"
#include "daqi/redis-client/redis_buffer.hpp"
#include "daqi/redis-client/redis_value.hpp"

namespace da4qi4
{

enum class RedisClientErrorHandlePolicy {do_nothing, auto_reconnect};

class RedisParser;
class RedisClient
    : public boost::noncopyable
{
public:
    RedisClient(IOC& ioc,
                RedisClientErrorHandlePolicy policy = RedisClientErrorHandlePolicy::do_nothing)
        : _socket(ioc), _reconnect_timer(ioc), _error_handle_policy(policy)
    {
    }

    ~RedisClient();

    void Connect(std::function<void (boost::system::error_code const& ec)> on = nullptr,
                 std::string const& host = "127.0.0.1", unsigned short port = 6379);

    void Reconnect(std::function<void (boost::system::error_code const& ec)> on = nullptr);

    bool ConnectSync(std::string const& host = "127.0.0.1", unsigned short port = 6379);
    bool ReconnectSync()
    {
        if (IsConnected())
        {
            do_disconnect();
            return do_sync_connect();
        }

        return true;
    }
    void Disconnect();

    RedisValue CommandSync(std::string cmd, std::deque<RedisBuffer> args);
    void Command(std::string cmd, std::deque<RedisBuffer> args,
                 std::function<void(RedisValue value)> on = nullptr);

    bool IsConnected() const
    {
        return _connect_status == is_connected;
    }

    bool IsConnectting() const
    {
        return _connect_status == is_connectting;
    }

private:
    void do_disconnect();
    bool do_sync_connect();
    void do_async_connect(std::function<void (boost::system::error_code const& ec)> on);
    void on_connect_finished(std::function<void (boost::system::error_code const& ec)> on
                             , boost::system::error_code const& ec);
    bool start_reconnect_timer(std::function<void (boost::system::error_code const& ec)> on);

    void start_aysnc_read_and_parse(std::function<void(RedisValue value)> on);
    void do_aysnc_read_and_parse(std::shared_ptr<RedisParser> parser, std::function<void(RedisValue value)> on);

private:
    void start_async_write();

private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::deadline_timer _reconnect_timer;
    RedisClientErrorHandlePolicy _error_handle_policy;
    std::string _host;
    unsigned short _port = 0;
    unsigned int _reconnect_count = 0;

private:
    using CommandNode = std::pair<std::vector<char>, std::function<void(RedisValue value)>>;
    std::queue<CommandNode> _command_queue;

private:
    std::string _reply_buf;
    std::size_t _reply_parse_beg;
    static size_t const  _read_buffer_size_ = 512;
    char _tmp_read_buf[_read_buffer_size_];

private:
    enum ConnectStatus {not_connect, is_connectting, is_connected};
    ConnectStatus _connect_status = not_connect;
};

using RedisClientPtr = std::shared_ptr<RedisClient>;

} // namespace da4qi4

#endif // DAQI_REDIS_CLIENT_HPP
