#ifndef DAQI_REDIS_COMMAND_HPP
#define DAQI_REDIS_COMMAND_HPP

#include <vector>
#include <deque>

#include "daqi/redis-client/redis_buffer.hpp"

namespace da4qi4
{

std::vector<char> MakeCommand(const std::deque<RedisBuffer>& items);

} // namespace da4qi4

#endif // DAQI_REDIS_COMMAND_HPP
