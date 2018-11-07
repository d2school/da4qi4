#ifndef DAQI_INTERCEPTER_SESSION_REDIS_HPP
#define DAQI_INTERCEPTER_SESSION_REDIS_HPP

#include <string>
#include <mutex>
#include <unordered_map>

#include "def/def.hpp"
#include "def/redis_def.hpp"
#include "def/inja_def.hpp"
#include "def/asio_def.hpp"
#include "def/boost_def.hpp"

#include "intercepter.hpp"

#include "session.hpp"
#include "session_redis_client.hpp"

namespace da4qi4
{

namespace Intercepter
{

/*
struct SessionRedisIntercepter
{
    SessionRedisIntercepter();


private:
    SessionRedisClient _client;
    SessionOptions _options;
};

*/
} //namespace Intercepter
} //namespace da4qi4

#endif // DAQI_INTERCEPTER_SESSION_REDIS_HPP
