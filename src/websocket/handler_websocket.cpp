#include "daqi/websocket/handler_websocket.hpp"

namespace da4qi4
{
namespace Websocket
{

bool EventHandleFunctor::OnOpen(Context ctx)
{
    return (OnOpenFunctor) ? OnOpenFunctor(ctx) : true;
}

void EventHandleFunctor::OnText(Context ctx, std::string&& data, bool is_finished)
{
    if (OnTextFunctor)
    {
        OnTextFunctor(ctx, std::move(data), is_finished);
    }
}

void EventHandleFunctor::OnBinary(Context ctx, std::string&& data, bool is_finished)
{
    if (OnBinaryFunctor)
    {
        OnBinaryFunctor(ctx, std::move(data), is_finished);
    }
}

void EventHandleFunctor::OnError(Context ctx, EventOn evt, int code, std::string const& msg)
{
    if (OnErrorFunctor)
    {
        OnErrorFunctor(ctx, evt, code, msg);
    }
}

void EventHandleFunctor::OnClose(Context ctx, EventOn evt)
{
    if (OnCloseFunctor)
    {
        OnCloseFunctor(ctx, evt);
    }
}

EventsHandler* EventHandleFunctor::operator()(void)
{
    return new EventHandleFunctor(*this);
}
} //namespace Websocket
} //namespace da4qi4
