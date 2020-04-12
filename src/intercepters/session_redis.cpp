#include "daqi/intercepters/session_redis.hpp"

#include <ctime>
#include <string>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "daqi/utilities/asio_utilities.hpp"
#include "daqi/utilities/string_utilities.hpp"

#include "daqi/context.hpp"

namespace da4qi4
{
namespace Intercepter
{

std::string make_session_id(std::string const& prefix, bool prefix_with_time)
{
    if (!prefix_with_time)
    {
        return Utilities::GetUUID(prefix);
    }

    char buf[16];
    auto now = std::time(nullptr);
    std::tm tm_now = *std::localtime(&now);
    std::strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", &tm_now);
    buf[15] = '\0';
    std::string new_prefix = prefix + std::string(buf) + ":";

    return Utilities::GetUUID(new_prefix);
}

Json SessionOnRedis::create_new_session() const
{
    std::string session_id = make_session_id(_options.prefix, _options.prefix_with_time);

    Cookie cookie(_options.name, session_id, _options.domain, _options.path);
    cookie.SetMaxAge(_options.max_age);
    cookie.SetHttpOnly(_options.http_only);
    cookie.SetSecure(_options.secure);
    cookie.SetSameSite(_options.samesite);

    return MakeNewSession(cookie);
}

void SessionOnRedis::on_request(Context& ctx) const
{
    std::string session_id = ctx->Req().GetCookie(this->_options.name);

    if (session_id.empty())
    {
        ctx->SaveSessionData(create_new_session());
        ctx->Pass();
        return;
    }

    if (auto redis = ctx->Redis())
    {
        boost::asio::spawn(redis->Strand(), [session_id, ctx, this, redis](asio_yield_ctx ytx)
        {
              RedisValue value = redis->Command("GET", {session_id}, ytx);

              if (value.IsError())
              {
                  ctx->Logger()->error("Get session cache fail. {}. {}",
                                       session_id, value.ToString());

                  ctx->RenderInternalServerError();
                  ctx->Stop();
                  return;
              }

              Json session;

              try
              {
                  if (!value.ToString().empty())
                  {
                      session = Json::parse(value.ToString());
                  }

                  if (session.empty())
                  {
                      session = create_new_session();
                  }
              }
              catch (Json::parse_error const& e)
              {
                  ctx->Logger()->error("Parse session data exception. {}. {}", session_id, e.what());
                  ctx->RenderInternalServerError();
                  ctx->Stop();
              }
              catch (std::exception const& e)
              {
                  ctx->Logger()->error("Parse session data exception. {}. {}", session_id, e.what());
                  ctx->RenderInternalServerError();
                  ctx->Stop();
              }

              ctx->SaveSessionData(std::move(session));
              ctx->Pass();
          });
    }
}

void SessionOnRedis::on_response(Context& ctx) const
{
    Json const& session = ctx->LoadSessionData();

    if (session.empty())
    {
        ctx->Pass();
        return;
    }

    Cookie cookie = GetSessionCookie(session);

    if (cookie.IsEmpty())
    {
        ctx->Pass();
        return;
    }

    ctx->Res().SetCookie(cookie);

    std::string session_timeout_s = std::to_string(cookie.GetMaxAge());
    std::string session_id = cookie.GetValue();
    size_t const indent = 2;
    std::string session_value = session.dump(indent);

    if (auto redis = ctx->Redis())
    {
        boost::asio::spawn(redis->Strand(), [ctx, redis, session_id, session_timeout_s, session_value](asio_yield_ctx ytx)
        {
            RedisValue value = redis->Command("SETEX", {session_id, session_timeout_s, session_value}, ytx);

            if (value.IsError())
            {
                ctx->Logger()->critical("Cache session data fail. {}", value.ToString());

                ctx->RenderInternalServerError();
                ctx->Stop();
            }

            ctx->Pass();
        });
    }
}

void SessionOnRedis::operator()(Context ctx, On on) const
{
    if (_options.name.empty())
    {
        ctx->Pass();
        return;
    }

    if (!Utilities::iStartsWith(ctx->Req().GetUrl().path, _options.path))
    {
        ctx->Pass();
        return;
    }

    if (!ctx->HasRedis())
    {
        ctx->Pass();
        return;
    }

    if (on == Intercepter::On::Request)
    {
        on_request(ctx);
    }
    else
    {
        on_response(ctx);
    }
}

} //namespace Intercepter
} //namespace da4qi4
