#include "daqi/session.hpp"

namespace da4qi4
{

namespace
{
std::string session_cookie_name = "_cookie_";
}

Json MakeNewSession(Cookie const& cookie)
{
    Json session {{session_cookie_name, cookie}};
    return session;
}

Cookie GetSessionCookie(Json const& session)
{
    Cookie cookie;

    Json::const_iterator it = session.find(session_cookie_name);

    if (it != session.cend())
    {
        it->get_to(cookie);
    }

    return cookie;
}

} //namespace da4qi4
