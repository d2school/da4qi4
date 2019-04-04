#include "daqi/redis-client/redis_buffer.hpp"

namespace da4qi4
{

RedisBuffer::RedisBuffer(const char* ptr, size_t dataSize)
    : data(ptr + 0, ptr + dataSize)
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
    : data(buf.cbegin(), buf.cend())
{
}

size_t RedisBuffer::size() const
{
    return data.size();
}

} // namespace da4qi4
