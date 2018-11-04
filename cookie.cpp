#include "cookie.hpp"

#include "utilities/string_utilities.hpp"

namespace da4qi4
{
Cookie& Cookie::SetExpires(std::time_t a_time_point)
{
    std::time_t now(std::time(nullptr));
    _max_age = a_time_point - now;
    return *this;
}

struct QV
{
    QV(std::string const& v, char const* q = "")
        : v(v), q(q)
    {}

    std::string const& v;
    std::string q;
};

std::ostream& operator << (std::ostream& os, QV const& qv)
{
    if (qv.q.empty())
    {
        return os << qv.v;
    }

    bool need_esc = qv.v.find(qv.q) != std::string::npos;

    if (!need_esc)
    {
        return os << qv.q << qv.v << qv.q;
    }

    std::string dst = std::string("\\") + qv.q;
    std::string esc = Utilities::ReplaceAll(qv.v, qv.q, dst);
    return os << qv.q << esc << qv.q;
}

std::ostream& operator << (std::ostream& os, Cookie const& c)
{
    char const* qs[] = {"", "\""};
    char const* q = qs[!!c._old_version];
    os << c._name << '=' << QV(c._value, q);

    if (c.IsExpiredImmediately())
    {
        os << ((!c._old_version) ? "; Max-Age=-1" : "; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
    }
    else if (!c.IsExpiredAfterBrowerClose())
    {
        if (!c._old_version)
        {
            os << "; Max-Age=" << QV(std::to_string(c._max_age), q);
        }
        else
        {
            std::time_t expires = std::time(nullptr) + c._max_age;
            os << "; Expires=" << QV(Utilities::GMTFormatTime(expires), q);
        }
    }

    if (!c._domain.empty())
    {
        os << "; Domain=" << QV(c._domain, q);
    }

    if (!c._path.empty())
    {
        os << "; Path=" << QV(c._path, q);
    }

    if (!c._old_version)
    {
        if (c._samesite != Cookie::SameSite::none)
        {
            std::string v = (c._samesite == Cookie::SameSite::lax) ? "Lax" : "Strict";
            os << "; SameSite=" << QV(v, q);
        }
    }

    if (c._secure)
    {
        os << "; Secure";
    }

    if (c._http_only)
    {
        os << "; HttpOnly";
    }

    return os;
}
} //namespace da4qi4
