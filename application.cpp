#include "application.hpp"

#include "def/debug_def.hpp"

namespace da4qi4
{

static char const* default_app_name = "da4qi4-Default";

ApplicationMgr* AppMgr()
{
    static ApplicationMgr mgr;
    return &mgr;
}

Application::Application()
    : _name(default_app_name)
    , _default_charset("utf-8")
    , _root_url("/")
{
    init_pathes();
}

Application::Application(std::string const& name
                         , std::string const& root_url
                         , fs::path const& root_static
                         , fs::path const& root_template
                         , fs::path const& root_upload
                         , fs::path const& root_temporary)
    : Application(name, "utf-8", root_url, root_static, root_template, root_upload, root_temporary)
{
}

Application::Application(std::string const& name
                         , std::string const& default_charset
                         , std::string const& root_url
                         , fs::path const& root_static
                         , fs::path const& root_template
                         , fs::path const& root_upload
                         , fs::path const& root_temporary)
    : _name(name)
    , _default_charset(default_charset)
    , _root_url(root_url)
    , _root_static(root_static)
    , _root_template(root_template)
    , _root_upload(root_upload)
    , _root_temporary(root_temporary)
{
    init_pathes();
}

Application::~Application()
{
}

void Application::Disable()
{
    _disabled = true;
}

void Application::Enable()
{
    _disabled = false;
}


#define assign_path(dst, path_getter) \
    do \
    { \
        if (dst.empty()) \
        {\
            errorcode ec; \
            dst = path_getter(ec); \
            if (ec) \
            { \
                std::cerr << "init default path for " << #dst << " fail. " << ec << "\r\n"; \
            } \
        }\
    } while (false)

void Application::init_pathes()
{
    assign_path(_root_static, fs::current_path);
    assign_path(_root_template, fs::current_path);
    assign_path(_root_upload, fs::temp_directory_path);
    assign_path(_root_temporary, fs::temp_directory_path);
}

bool ApplicationMgr::CreateDefaultIfEmpty()
{
    if (_set.empty())
    {
        _set.insert(Application());
        return true;
    }

    return false;
}

bool ApplicationMgr::Add(Application const& app)
{
    if (!app.GetName().empty() && !IsExists(app.GetName()))
    {
        _set.insert(app);
        return true;
    }

    return false;
}

void ApplicationMgr::Enable(std::string const& name)
{
    auto app = this->FindByName(name);

    if (app && !app->IsEnable())
    {
        app->Enable();
    }
}

void ApplicationMgr::Disable(std::string const& name)
{
    auto app = FindByName(name);

    if (app && app->IsEnable())
    {
        app->Disable();
    }
}

bool ApplicationMgr::IsExists(std::string const& name) const
{
    return FindByName(name) != nullptr;
}

bool ApplicationMgr::IsEnable(std::string const& name) const
{
    auto app = FindByName(name);
    return app && app->IsEnable();
}

Application* ApplicationMgr::FindByURL(std::string const& url)
{
    auto dst(Application::ForCompareUrl(url));
    auto l = _set.upper_bound(dst);

    if (l == _set.end())
    {
        return nullptr;
    }

    if (Utilities::iStartsWith(url, l->GetUrlRoot()))
    {
        return const_cast<Application*>(&(*l));
    }

    auto u = _set.lower_bound(dst);

    for (auto it = ++l; it != u && it != _set.end(); ++it)
    {
        if (Utilities::iStartsWith(url, it->GetUrlRoot()))
        {
            return const_cast<Application*>(&(*l)); //(set's element is always const)
        }
    }

    return nullptr;
}

Application* ApplicationMgr::FindByName(std::string const& name)
{
    for (auto& a : _set)
    {
        if (a.GetName() == name)
        {
            return const_cast<Application*>(&a);
        }
    }

    return nullptr;
}

Application const* ApplicationMgr::FindByName(std::string const& name) const
{
    for (auto& a : _set)
    {
        if (a.GetName() == name)
        {
            return &a;
        }
    }

    return nullptr;
}

std::string join_app_path(std::string const& app_root, std::string const& path)
{
    if (!path.empty() && !app_root.empty())
    {
        if (*app_root.rbegin() == '/' && *path.begin() == '/')
        {
            return app_root + path.substr(1, path.size() - 1);
        }

        if (*app_root.rbegin() != '/' && *path.begin() != '/')
        {
            return app_root + "/" + path;
        }
    }

    return app_root + path;
}

bool Application::AddHandler(HandlerMethod m, router_equals r, Handler h)
{
    r.s = join_app_path(_root_url, r.s);
    return _equalRouter.Add(r, m, h);
}

bool Application::AddHandler(HandlerMethod m, router_starts r, Handler h)
{
    r.s = join_app_path(_root_url, r.s);
    return _startwithsRouter.Add(r, m, h);
}

bool Application::AddHandler(HandlerMethod m, router_regex r, Handler h)
{
    r.s = join_app_path(_root_url, r.s);
    return _regexRouter.Add(r, m, h);
}

bool Application::AddHandler(HandlerMethods ms, router_equals r, Handler h)
{
    r.s = join_app_path(_root_url, r.s);
    return _equalRouter.Add(r, ms, h);
}

bool Application::AddHandler(HandlerMethods ms, router_starts r, Handler h)
{
    r.s = join_app_path(_root_url, r.s);
    _startwithsRouter.Add(r, ms, h);
}

bool Application::AddHandler(HandlerMethods ms, router_regex r, Handler h)
{
    r.s = join_app_path(_root_url, r.s);
    return _regexRouter.Add(r, ms, h);
}

Handler Application::FindHandler(Context ctx)
{
    HandlerMethod m = from_http_method((http_method)ctx->Req().GetMethod());

    if (m == HandlerMethod::UNSUPPORT)
    {
        return theEmptyHandler;
    }

    std::string const& url = ctx->Req().GetUrl().full;
    Handler h = _equalRouter.Search(url, m);

    if (!h)
    {
        h = _startwithsRouter.Search(url, m);

        if (!h)
        {
            h = _regexRouter.Search(url, m);
        }
    }

    return h;
}

void Application::Handle(Context ctx)
{
    Handler h = FindHandler(ctx);

    if (!h)
    {
        //404
        ctx->Res().Nofound();
        ctx->Bye();
        return;
    }

    h(ctx);
}

} //namespace da4qi4
