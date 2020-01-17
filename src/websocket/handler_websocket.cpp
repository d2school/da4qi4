#include "daqi/websocket/handler_websocket.hpp"

namespace da4qi4
{
namespace Websocket
{

bool EventHandleFunctor::OnOpen(Context ctx)
{
    return (DoOnOpen) ? DoOnOpen(ctx) : true;
}

void EventHandleFunctor::OnText(Context ctx, std::string&& data, bool is_finished)
{
    if (DoOnText)
    {
        DoOnText(ctx, std::move(data), is_finished);
    }
}

void EventHandleFunctor::OnBinary(Context ctx, std::string&& data, bool is_finished)
{
    if (DoOnBinary)
    {
        DoOnBinary(ctx, std::move(data), is_finished);
    }
}

void EventHandleFunctor::OnError(Context ctx, EventOn evt, int code, std::string const& msg)
{
    if (DoOnError)
    {
        DoOnError(ctx, evt, code, msg);
    }
}

void EventHandleFunctor::OnClose(Context ctx, EventOn evt)
{
    if (DoOnClose)
    {
        DoOnClose(ctx, evt);
    }
}

EventsHandler* EventHandleFunctor::operator()(void)
{
    return new EventHandleFunctor(*this);
}
} //namespace Websocket
} //namespace da4qi4
