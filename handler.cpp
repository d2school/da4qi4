#include "handler.hpp"

namespace da4qi4
{

Handler theEmptyHandler;
Handler theMethodUnsupportHandler;

HandlerMethod _DELETE_ = HandlerMethod::DELETE;
HandlerMethod _GET_ = HandlerMethod ::GET;
HandlerMethod _HEAD_ = HandlerMethod::HEAD;
HandlerMethod _POST_ = HandlerMethod::POST;
HandlerMethod _PUT_ = HandlerMethod::PUT;

HandlerMethod from_http_method(http_method m)
{
    switch (m)
    {
        case HTTP_DELETE :
            return HandlerMethod::DELETE;

        case HTTP_GET :
            return HandlerMethod::GET;

        case HTTP_HEAD :
            return HandlerMethod::HEAD;

        case HTTP_POST :
            return HandlerMethod::POST;

        case HTTP_PUT :
            return HandlerMethod::PUT;

        default:
            break;
    }

    return HandlerMethod::UNSUPPORT;
}


HandlerMethods::HandlerMethods(HandlerMethod m)
{
    Set(m);
}

bool HandlerMethods::IsSet(HandlerMethod m) const
{
    switch (m)
    {
        case HandlerMethod::DELETE :
            return mark[0];

        case HandlerMethod::GET :
            return mark[1];

        case HandlerMethod::HEAD :
            return mark[2];

        case HandlerMethod::POST :
            return mark[3];

        case HandlerMethod::PUT :
            return mark[4];

        case HandlerMethod::ANY :
            return mark.any();

        case HandlerMethod::UNSUPPORT :
            return !mark.any();
    }

    return false;
}

void HandlerMethods::Set(HandlerMethod m)
{
    switch (m)
    {
        case HandlerMethod::DELETE :
            mark.set(0);
            break;

        case HandlerMethod::GET :
            mark.set(1);
            break;

        case HandlerMethod::HEAD :
            mark.set(2);
            break;

        case HandlerMethod::POST :
            mark.set(3);
            break;

        case HandlerMethod::PUT :
            mark.set(4);
            break;

        case HandlerMethod::ANY :
            mark.reset();
            mark.flip();

        case HandlerMethod::UNSUPPORT :
            mark.reset();
    }
}

HandlerMethods::HandlerMethods(std::initializer_list<HandlerMethod> lst)
{
    for (auto m : lst)
    {
        Set(m);
    }
}



} //namespace da4qi4
