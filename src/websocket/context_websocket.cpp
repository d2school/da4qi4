#include "daqi/websocket/context_websocket.hpp"

#include <cstring>

#include "daqi/application.hpp"

namespace da4qi4
{
namespace Websocket
{

ContextIMP::ContextIMP(bool hold_connection_life, Connection::Ptr cnt, ApplicationPtr app)
    : _hold_connection_life(hold_connection_life), _cnt(cnt), _app(app)
{
    assert(_cnt != nullptr);
    assert(_app != nullptr);
}

Context ContextIMP::Create(Connection::Ptr cnt, ApplicationPtr app)
{
    return Context(new ContextIMP(true, cnt, app));
}

Context ContextIMP::CreateUnholdConnectionLife(Connection::Ptr cnt, ApplicationPtr app)
{
    return Context(new ContextIMP(false, cnt, app));
}

ContextIMP::~ContextIMP()
{
    if (_hold_connection_life)
    {
        auto fullpath = _cnt->GetURLPath();

        if (!fullpath.empty() && !_cnt->GetID().empty())
        {
            _app->RemoveWebSocketConnection(fullpath, UrlFlag::url_full_path, _cnt->GetID());
        }
    }
}

IOC& ContextIMP::IOContext()
{
    return _cnt->GetIOC();
}

size_t ContextIMP::IOContextIndex() const
{
    return _cnt->GetIOContextIndex();
}

bool operator == (ContextIMP const& o1, ContextIMP const& o2)
{
    return (o1._cnt == o2._cnt);
}

bool operator != (ContextIMP const& o1, ContextIMP const& o2)
{
    return !(o1 == o2);
}

bool ContextIMP::RenameID(std::string const& new_id)
{
    return _app->RenameWebSocketConnectionID(_cnt->GetURLPath(), UrlFlag::url_full_path, _cnt->GetID(), new_id);
}

log::LoggerPtr ContextIMP::Logger()
{
    return _app->GetLogger();
}

Application& ContextIMP::App()
{
    return *_app;
}

Application const& ContextIMP::App() const
{
    return *_app;
}

void ContextIMP::SendText(std::string const& str)
{
    _cnt->Write(shared_from_this(), str, WriteDataType::text, true);
}

void ContextIMP::SendText(char const* str, std::size_t len)
{
    std::size_t sz = (len != static_cast<std::size_t>(-1)) ? len : std::strlen(str);
    std::string s(str, sz);
    SendText(s);
}

void ContextIMP::SendFirstText(std::string const& str)
{
    _cnt->Write(shared_from_this(), str, WriteDataType::text, false);
}
void ContextIMP::SendNextText(std::string const& str)
{
    _cnt->Write(shared_from_this(), str, WriteDataType::continuation, false);
}
void ContextIMP::SendLastText(std::string const& str)
{
    _cnt->Write(shared_from_this(), str, WriteDataType::continuation, true);
}

void ContextIMP::SendFirstText(char const* str, std::size_t len)
{
    std::size_t sz = (len != static_cast<std::size_t>(-1)) ? len : std::strlen(str);
    std::string s(str, sz);
    SendFirstText(s);
}
void ContextIMP::SendNextText(char const* str, std::size_t len)
{
    std::size_t sz = (len != static_cast<std::size_t>(-1)) ? len : std::strlen(str);
    std::string s(str, sz);
    SendNextText(s);
}
void ContextIMP::SendLastText(char const* str, std::size_t len)
{
    std::size_t sz = (len != static_cast<std::size_t>(-1)) ? len : std::strlen(str);
    std::string s(str, sz);
    SendLastText(s);
}

void ContextIMP::SendBinary(std::vector<std::int8_t> const& data)
{
    std::string s(data.cbegin(), data.cend());
    _cnt->Write(shared_from_this(), s, WriteDataType::binary, true);
}

void ContextIMP::SendBinary(char const* data, std::size_t size)
{
    std::string s(data, size);
    _cnt->Write(shared_from_this(), s, WriteDataType::binary, true);
}

void ContextIMP::SendFirstBinary(std::vector<std::int8_t> const& data)
{
    std::string s(data.cbegin(), data.cend());
    _cnt->Write(shared_from_this(), s, WriteDataType::binary, false);
}
void ContextIMP::SendNextBinary(std::vector<std::int8_t> const& data)
{
    std::string s(data.cbegin(), data.cend());
    _cnt->Write(shared_from_this(), s, WriteDataType::continuation, false);
}
void ContextIMP::SendLastBinary(std::vector<std::int8_t> const& data)
{
    std::string s(data.cbegin(), data.cend());
    _cnt->Write(shared_from_this(), s, WriteDataType::continuation, true);
}

void ContextIMP::SendFirstBinary(char const* data, std::size_t size)
{
    std::string s(data, size);
    _cnt->Write(shared_from_this(), s, WriteDataType::binary, false);
}
void ContextIMP::SendNextBinary(char const* data, std::size_t size)
{
    std::string s(data, size);
    _cnt->Write(shared_from_this(), s, WriteDataType::continuation, false);
}
void ContextIMP::SendLastBinary(char const* data, std::size_t size)
{
    std::string s(data, size);
    _cnt->Write(shared_from_this(), s, WriteDataType::continuation, true);
}

Context ContextIMP::OtherOne(std::string const& id)
{
    if (id == _cnt->GetID())
    {
        return this->shared_from_this();
    }

    auto cnt = _app->WebsocketConnection(_cnt->GetURLPath(), UrlFlag::url_full_path, id);
    return cnt ? CreateUnholdConnectionLife(cnt, _app) : nullptr;
}

Context ContextIMP::OtherOne(std::string const& url, UrlFlag url_flag, std::string const& id)
{
    if (id == _cnt->GetID() && (MakesureFullUrlPath(url, url_flag, _app->GetUrlRoot()) == _cnt->GetURLPath()))
    {
        return this->shared_from_this();
    }

    auto cnt = _app->WebsocketConnection(url, url_flag, id);
    return cnt ? CreateUnholdConnectionLife(cnt, _app) : nullptr;
}

Context ContextIMP::OtherOne(ApplicationPtr other_app, std::string const& url, UrlFlag url_flag, std::string const& id)
{
    if ((other_app == _app) && (id == _cnt->GetID())
        && (MakesureFullUrlPath(url, url_flag, _app->GetUrlRoot()) == _cnt->GetURLPath()))
    {
        return this->shared_from_this();
    }

    auto cnt = other_app->WebsocketConnection(url, url_flag, id);
    return cnt ? CreateUnholdConnectionLife(cnt, other_app) : nullptr;
}

ContextList::ContextList(ApplicationPtr app, std::string const& url, UrlFlag flag)
    : _app(app), _cnt_list(_app->AllWebSocketConnections(url, flag))
{
}

ContextList::ContextList(Context src_ctx, std::string const& url, UrlFlag flag)
    : _app(src_ctx->AppPtr()), _cnt_list(_app->AllWebSocketConnections(url, flag))
{
}

ContextList::ContextList(Context src_ctx)
    : _app(src_ctx->AppPtr()), _cnt_list(_app->AllWebSocketConnections(src_ctx->URLPath(), UrlFlag::url_full_path))
{
}

ContextList::iterator ContextList::begin()
{
    return iterator(_cnt_list.begin(), _app);
}

ContextList::iterator ContextList::end()
{
    std::weak_ptr<Application> nullapp;
    return iterator(_cnt_list.end(), nullapp);
}

ContextList::const_iterator ContextList::cbegin() const
{
    return const_iterator(_cnt_list.cbegin(), _app);
}

ContextList::const_iterator ContextList::cend() const
{
    std::weak_ptr<Application> nullapp;
    return const_iterator(_cnt_list.cend(), nullapp);
}

} // namespace Websocket
} // namespace da4qi4
