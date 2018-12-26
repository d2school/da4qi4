#include "redis-client/redis_buffer.hpp"

namespace da4qi4
{

RedisBuffer::RedisBuffer(const char* ptr, size_t dataSize)
    : data(std::vector<char>(ptr, ptr + dataSize))
{
}

RedisBuffer::RedisBuffer(const char* s)
    : data(std::string(s))
{
}

RedisBuffer::RedisBuffer(std::string s)
    : data(std::move(s))
{
}

RedisBuffer::RedisBuffer(std::vector<char> buf)
    : data(std::move(buf))
{
}

size_t RedisBuffer::size() const
{
    if (data.type() == typeid(std::string))
    {
        return boost::get<std::string>(data).size();
    }
    else
    {
        return boost::get<std::vector<char>>(data).size();
    }
}

} // namespace da4qi4
