#include "daqi/redis-client/redis_command.hpp"

#include <cstring>
#include <string>

#include <boost/variant.hpp>

namespace da4qi4
{

namespace
{

static const char crlf[] = {'\r', '\n'};

void bufferAppend(std::vector<char>& vec, std::string const& s);
void bufferAppend(std::vector<char>& vec, char c);

template<std::size_t size>
void bufferAppend(std::vector<char>& vec, const char (&s)[size]);

void bufferAppend(std::vector<char>& vec, RedisBuffer const& buf)
{
    bufferAppend(vec, buf.data);
}

void bufferAppend(std::vector<char>& vec, std::string const& s)
{
    vec.insert(vec.end(), s.begin(), s.end());
}

void bufferAppend(std::vector<char>& vec, char c)
{
    vec.resize(vec.size() + 1);
    vec[vec.size() - 1] = c;
}

template<size_t size>
void bufferAppend(std::vector<char>& vec, const char (&s)[size])
{
    vec.insert(vec.end(), s, s + size);
}

} //namespace


std::vector<char> MakeCommand(const std::deque<RedisBuffer>& items)
{
    std::vector<char> result;
    result.reserve(128);

    bufferAppend(result, '*');
    bufferAppend(result, std::to_string(items.size()));
    bufferAppend<>(result, crlf);

    for (const auto& item : items)
    {
        bufferAppend(result, '$');
        bufferAppend(result, std::to_string(item.size()));
        bufferAppend<>(result, crlf);
        bufferAppend(result, item);
        bufferAppend<>(result, crlf);
    }

    return result;
}

} // namespace da4qi4
