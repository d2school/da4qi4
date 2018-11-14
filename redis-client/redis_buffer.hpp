#ifndef DAQI_REDIS_BUFFER_H
#define DAQI_REDIS_BUFFER_H

#include <string>
#include <boost/variant.hpp>

namespace da4qi4
{

struct RedisBuffer
{
    RedisBuffer() = default;
    RedisBuffer(const char* ptr, size_t dataSize);
    RedisBuffer(const char* s);
    RedisBuffer(std::string s);
    RedisBuffer(std::vector<char> buf);

    template<typename SrcT>
    RedisBuffer(SrcT const& value)
        : data(std::move(std::to_string(value)))
    {}

    size_t size() const;

    boost::variant<std::string, std::vector<char>> data;
};

} // namespace da4qi4

#endif // DAQI_REDIS_BUFFER_H
