#ifndef HANDLER_WEBSOCKET_HPP
#define HANDLER_WEBSOCKET_HPP

#include <functional>
#include <memory>

namespace da4qi4
{
namespace Websocket
{

class ContextIMP;
typedef std::shared_ptr<ContextIMP> Context;

enum class EventOn
{
    read_event, write_event,
};

class EventsHandler
{
public:
    virtual ~EventsHandler() = default;

    virtual bool OnOpen(Context ctx) = 0;

    virtual void OnText(Context ctx, std::string&& data, bool finished) = 0;
    virtual void OnBinary(Context ctx, std::string&& data, bool finished) = 0;

    virtual void OnError(Context ctx, EventOn evt, int code, std::string const& error) = 0;
    virtual void OnClose(Context ctx, EventOn evt) = 0;
};

using EventHandlersFactory = std::function<EventsHandler * (void)>;

class EmptyEventHandler : public EventsHandler
{
public:
    EmptyEventHandler() = default;
    ~EmptyEventHandler() override = default;

    bool OnOpen(Context) override
    {
        return true;
    }

    void OnText(Context, std::string&&, bool) override {}
    void OnBinary(Context, std::string&&, bool) override {}
    void OnError(Context, EventOn, int, std::string const&) override {}
    void OnClose(Context, EventOn) override {}
};

struct EventHandleFunctor : public EventsHandler
{
    std::function < bool (Context) > OnOpenFunctor;
    std::function < void (Context, std::string&&, bool) > OnTextFunctor;
    std::function < void (Context, std::string&&, bool) > OnBinaryFunctor;
    std::function < void (Context, EventOn, int, std::string const&) > OnErrorFunctor;
    std::function < void (Context, EventOn) > OnCloseFunctor;

    bool OnOpen(Context ctx) override;
    void OnText(Context ctx, std::string&& data, bool is_finished) override;
    void OnBinary(Context ctx, std::string&& data, bool is_finished) override;
    void OnError(Context ctx, EventOn evt, int code, std::string const& msg) override;
    void OnClose(Context ctx, EventOn evt) override;

    EventsHandler* operator()(void);
};

} //namespace Websocket
} //namespace da4qi4

#endif // HANDLER_WEBSOCKET_HPP
