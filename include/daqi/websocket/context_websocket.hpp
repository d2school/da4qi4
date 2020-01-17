#ifndef CONTEXT_WEBSOCKET_HPP
#define CONTEXT_WEBSOCKET_HPP

#include <functional>

#include "daqi/def/log_def.hpp"
#include "daqi/websocket/connection_websocket.hpp"

namespace da4qi4
{

class Application;
using ApplicationPtr = std::shared_ptr<Application>;

namespace Websocket
{

class ContextIMP final : public std::enable_shared_from_this<ContextIMP>
{
private:
    ContextIMP(bool hold_connection_life, Connection::Ptr cnt, ApplicationPtr app);

public:
    static Context Create(Connection::Ptr cnt, ApplicationPtr app);
    static Context CreateUnholdConnectionLife(Connection::Ptr cnt, ApplicationPtr app);

    ~ContextIMP();

    IOC& IOContext();
    size_t IOContextIndex() const;

public:
    Application& App();
    Application const& App() const;

    ApplicationPtr AppPtr()
    {
        return _app;
    }

    log::LoggerPtr Logger();

public:
    void SendText(std::string const& str);
    void SendText(char const* str, std::size_t len = static_cast<std::size_t>(-1));

    void SendFirstText(std::string const& str);
    void SendNextText(std::string const& str);
    void SendLastText(std::string const& str);

    void SendFirstText(char const* str, std::size_t len = static_cast<std::size_t>(-1));
    void SendNextText(char const* str, std::size_t len = static_cast<std::size_t>(-1));
    void SendLastText(char const* str, std::size_t len = static_cast<std::size_t>(-1));

    void SendBinary(std::vector<std::int8_t> const& data);
    void SendBinary(char const* data, std::size_t size);
    void SendBinary(std::string const& data)
    {
        SendBinary(data.c_str(), data.size());
    }

    void SendFirstBinary(std::vector<std::int8_t> const& data);
    void SendNextBinary(std::vector<std::int8_t> const& data);
    void SendLastBinary(std::vector<std::int8_t> const& data);

    void SendFirstBinary(char const* data, std::size_t size);
    void SendNextBinary(char const* data, std::size_t size);
    void SendLastBinary(char const* data, std::size_t size);

public:
    std::string URLPath() const
    {
        return _cnt->GetURLPath();
    }

    std::string const& ID() const
    {
        return _cnt->GetID();
    }

    bool RenameID(std::string const& new_id);

    Url GetURL() const
    {
        auto p = _cnt->GetURLDataPtr();
        return (p) ? *p : Url();
    }

    ICHeaders const& Headers() const
    {
        return _cnt->GetHeaders();
    }

    ICCookies const& Cookies() const
    {
        return _cnt->GetCookies();
    }

    void RemoveHTTPInfo()
    {
        _cnt->RemoveHttpInfo();
    }

public:
    bool IsHoldConnectionLife() const
    {
        return _hold_connection_life;
    }

public:
    Context OtherOne(std::string const& id);
    Context OtherOne(std::string const& url, UrlFlag url_flag, std::string const& id);
    Context OtherOne(ApplicationPtr other_app, std::string const& url, UrlFlag url_flag, std::string const& id);

private:
    bool _hold_connection_life;
    Connection::Ptr _cnt;
    ApplicationPtr _app;

    friend bool operator == (ContextIMP const& o1, ContextIMP const& o2);
};

bool operator == (ContextIMP const& o1, ContextIMP const& o2);
bool operator != (ContextIMP const& o1, ContextIMP const& o2);

class ContextList
{
public:
    struct iterator
    {
        iterator(std::list<Connection::Ptr>::iterator lst_it, std::weak_ptr<Application> app)
            : _lst_iterator(lst_it), _app(app)
        {}

        iterator& operator = (iterator const& o)
        {
            _lst_iterator = o._lst_iterator;
            return *this;
        }

        bool operator != (iterator const& o)
        {
            return _lst_iterator != o._lst_iterator;
        }

        iterator& operator ++()
        {
            ++_lst_iterator;
            return *this;
        }

        iterator operator ++(int)
        {
            auto tmp = _lst_iterator;
            _lst_iterator++;

            return iterator(tmp, _app);
        }

        Context operator * ()
        {
            return (!_app.lock()) ? nullptr : ContextIMP::CreateUnholdConnectionLife(*_lst_iterator, _app.lock());
        }

    private:
        std::list<Connection::Ptr>::iterator _lst_iterator;
        std::weak_ptr<Application> _app;
    };

    struct const_iterator
    {
        const_iterator(std::list<Connection::Ptr>::const_iterator lst_it, std::weak_ptr<Application> app)
            : _lst_iterator(lst_it), _app(app)
        {}

        const_iterator& operator = (const_iterator const& o)
        {
            _lst_iterator = o._lst_iterator;
            return *this;
        }

        bool operator != (const_iterator const& o)
        {
            return _lst_iterator != o._lst_iterator;
        }

        const_iterator& operator ++()
        {
            ++_lst_iterator;
            return *this;
        }

        const_iterator operator ++(int)
        {
            auto tmp = _lst_iterator;
            _lst_iterator++;

            return const_iterator(tmp, _app);
        }

        const Context operator * ()
        {
            return (!_app.lock()) ? nullptr : ContextIMP::CreateUnholdConnectionLife(*_lst_iterator, _app.lock());
        }

    private:
        std::list<Connection::Ptr>::const_iterator _lst_iterator;
        std::weak_ptr<Application> _app;
    };

public:
    ContextList(ApplicationPtr app, std::string const& url, UrlFlag flag);
    ContextList(Context src_ctx, std::string const& url, UrlFlag flag);
    ContextList(Context src_ctx);

    iterator begin();
    iterator end();

    const_iterator cbegin() const;
    const_iterator cend() const;

private:
    ApplicationPtr _app;
    std::list<Connection::Ptr> _cnt_list;
};

} // namespace Websocket
} // namespace da4qi4

#endif // CONTEXT_WEBSOCKET_HPP
