#ifndef DAQI_APPLICATION_HPP
#define DAQI_APPLICATION_HPP

#include <set>

#include "def/boost_def.hpp"
#include "utilities/string_utilities.hpp"

#include "router.hpp"

namespace da4qi4
{

class Application
{
private:
    Application(std::string const& root_url)
        : _name(""), _root_url(root_url)
    {}

public:
    static Application ForCompareUrl(std::string const& url)
    {
        return Application(url);
    }

    static Application& EmptyApplication()
    {
        static Application emptyApp("");
        return emptyApp;
    }

public:
    Application();

    Application(std::string const& name
                , std::string const& root_url
                , fs::path const& root_static = ""
                , fs::path const& root_template = ""
                , fs::path const& root_upload = ""
                , fs::path const& root_temporary = ""
               );

    Application(Application const&) = default;
    Application& operator = (Application const&) = delete;

    ~Application();

    Application& SetStaticRoot(std::string const& root_static)
    {
        _root_static = root_static;
        return *this;
    }

    Application& SetTemplateRoot(std::string const& root_template)
    {
        _root_template = root_template;
        return *this;
    }

    Application& SetDefaultCharset(std::string const& charset)
    {
        _default_charset = charset;
        return *this;
    }

    Application& SetUploadRoot(std::string const& root_upload)
    {
        _root_upload = root_upload;
        return *this;
    }

    std::string const& GetName() const
    {
        return _name;
    }

    std::string const& GetDefaultCharset() const
    {
        return _default_charset;
    }

    std::string const& GetUrlRoot() const
    {
        return _root_url;
    }

    fs::path const& GetStaticRootPath() const
    {
        return _root_static;
    }

    fs::path const& GetTemplateRoot() const
    {
        return _root_template;
    }

    fs::path const& GetUploadRoot() const
    {
        return _root_upload;
    }

    bool IsEnable() const
    {
        return !_disabled;
    }
    void Disable();
    void Enable();

    UploadFileSaveOptions const& GetUploadFileSaveOptions() const
    {
        return _upload_file_save_opt;
    }

    UploadFileSaveOptions& GetUploadFileSaveOptions()
    {
        return _upload_file_save_opt;
    }

    void Handle(Context ctx);

    void operator()(Context ctx)
    {
        Handle(ctx);
    }

public:
    bool AddHandler(HandlerMethod m, std::string const& url, Handler h)
    {
        return AddHandler(m, router_equals(url), h);
    }

    bool AddHandler(HandlerMethod m, router_equals r, Handler h);
    bool AddHandler(HandlerMethod m, router_starts r, Handler h);
    bool AddHandler(HandlerMethod m, router_regex r, Handler h);

    bool AddHandler(HandlerMethods ms, std::string const& url, Handler h)
    {
        return AddHandler(ms, router_equals(url), h);
    }
    bool AddHandler(HandlerMethods ms, router_equals e, Handler h);
    bool AddHandler(HandlerMethods ms, router_starts e, Handler h);
    bool AddHandler(HandlerMethods ms, router_regex e, Handler h);

private:
    void init_pathes();

private:
    Handler FindHandler(Context ctx);

private:
    EqualsRoutingTable _equalRouter;
    StartsWithRoutingTable _startwithsRouter;
    RegexMatchRoutingTable _regexRouter;

private:
    bool _disabled = false;

    std::string _name;
    std::string _default_charset = "UTF-8";

    std::string _root_url;
    fs::path _root_static;
    fs::path _root_template;
    fs::path _root_upload;
    fs::path _root_temporary;

    UploadFileSaveOptions _upload_file_save_opt;
};

struct CompareByUrlRoot_DESC_IC
{
    bool operator()(Application const& app1, Application const& app2)
    {
        return !Utilities::iLess(app1.GetUrlRoot(), app2.GetUrlRoot());
    }
};

using ApplicationSet = std::set <Application, CompareByUrlRoot_DESC_IC>;

struct ApplicationMgr
{
    bool CreateDefaultIfEmpty();
    bool Add(Application const& app);

    void Enable(std::string const& name);
    void Disable(std::string const& name);

    bool IsExists(std::string const& name) const;
    bool IsEnable(std::string const& name) const;

    Application* FindByURL(std::string const& url);

    Application* FindByName(std::string const& name);
    Application const* FindByName(std::string const& name) const;

    ApplicationSet const& All() const
    {
        return _set;
    }

    bool IsEmpty() const
    {
        return _set.empty();
    }
    size_t Count() const
    {
        return _set.size();
    }

private:
    ApplicationSet _set;
};

ApplicationMgr* AppMgr();

} // namespace da4qi4

#endif // DAQI_APPLICATION_HPP
