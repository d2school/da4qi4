#ifndef DAQI_HANDLER_HPP
#define DAQI_HANDLER_HPP

#include "llhttp/llhttp.h"

#include "daqi/context.hpp"

#include <bitset>
#include <initializer_list>

namespace da4qi4
{

enum class HandlerMethod
{
    UNSUPPORT = -1,

    DELETE = 0,
    GET,
    HEAD,
    POST,
    PUT,

    ANY = 999
};

extern HandlerMethod _DELETE_, _GET_, _HEAD_, _POST_, _PUT_;

char const* HandlerMethodName(HandlerMethod hm);

HandlerMethod from_http_method(llhttp_method_t m);

using HandlerMethodMark = std::bitset<5>;

struct HandlerMethods
{
    HandlerMethods() = default;

    HandlerMethods(HandlerMethod m);
    HandlerMethods(HandlerMethodMark mark)
        : mark(mark)
    {
    }

    HandlerMethods(std::initializer_list<HandlerMethod> lst);

    operator HandlerMethodMark() const
    {
        return mark;
    }

    void Set(HandlerMethod m);
    bool IsSet(HandlerMethod m) const;

    HandlerMethodMark mark;
};

using Handler = std::function<void (Context)>;

extern Handler theEmptyHandler;

template<typename C>
Handler member_handler(C* o, void (C::*f)(Context))
{
    return std::bind(f, o, std::placeholders::_1);
}

} //namespace da4qi4

#endif // DAQI_HANDLER_HPP
