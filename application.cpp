#include "application.hpp"

#include "def/debug_def.hpp"

namespace da4qi4
{

static char const* default_app_name = "da4qi4-Default";

ApplicationMgr& AppMgr()
{
    static ApplicationMgr mgr;
    return mgr;
}

bool UploadFileSaveOptions::IsNeedSave(std::string const& extension
                                       , size_t filesize_kb) const
{
    if (strategy == always_save)
    {
        return true;
    }

    if (strategy == alway_no_save)
    {
        return false;
    }

    if (strategy == size_greater_than)
    {
        return filesize_kb > size_base_kb;
    }

    if (strategy == size_lesser_than)
    {
        return filesize_kb < size_base_kb;
    }

    if (strategy == extension_is)
    {
        return extensions.find(extension) != extensions.cend();
    }

    if (strategy == extension_is_not)
    {
        return extensions.find(extension) == extensions.cend();
    }

    return false;
}


Application::Application()
    : _name(default_app_name)
    , _default_charset("utf-8")
    , _root_url("/")
    , _templates("/")
{
    init_pathes();
}

Application::Application(std::string const& name
                         , std::string const& root_url
                         , fs::path const& root_static
                         , fs::path const& root_template
                         , fs::path const& root_upload
                         , fs::path const& root_temporary)
    : _name(name)
    , _root_url(root_url)
    , _root_static(root_static)
    , _root_template(root_template)
    , _root_upload(root_upload)
    , _root_temporary(root_temporary)
    , _templates(root_template.native())

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

void Application::init_pathes()
{
    try
    {
        if (_root_template.empty())
        {
            _root_template = fs::current_path();
        }
        else if (!_root_template.is_absolute())
        {
            _root_template = fs::absolute(_root_template);
        }

        if (_root_upload.empty())
        {
            _root_upload = fs::temp_directory_path();
        }
        else if (!_root_upload.is_absolute())
        {
            _root_upload = fs::absolute(_root_upload);
        }

        if (_root_temporary.empty())
        {
            _root_temporary = fs::current_path();
        }
        else if (!_root_temporary.is_absolute())
        {
            _root_temporary = fs::absolute(_root_temporary);
        }
    }
    catch (fs::filesystem_error const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    _templates.Preload();
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

bool ApplicationMgr::Add(Application& app)
{
    if (!app.GetName().empty() && !IsExists(app.GetName()))
    {
        app._mounted = true;
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
    if (IsRuning())
    {
        return false;
    }

    r.s = join_app_path(_root_url, r.s);
    return _equalRouter.Add(r, m, h);
}

bool Application::AddHandler(HandlerMethod m, router_starts r, Handler h)
{
    if (IsRuning())
    {
        return false;
    }

    r.s = join_app_path(_root_url, r.s);
    return _startwithsRouter.Add(r, m, h);
}

bool Application::AddHandler(HandlerMethod m, router_regex r, Handler h)
{
    if (IsRuning())
    {
        return false;
    }

    r.s = join_app_path(_root_url, r.s);
    return _regexRouter.Add(r, m, h);
}

bool Application::AddHandler(HandlerMethods ms, router_equals r, Handler h)
{
    if (IsRuning())
    {
        return false;
    }

    r.s = join_app_path(_root_url, r.s);
    return _equalRouter.Add(r, ms, h);
}

bool Application::AddHandler(HandlerMethods ms, router_starts r, Handler h)
{
    if (IsRuning())
    {
        return false;
    }

    r.s = join_app_path(_root_url, r.s);
    return _startwithsRouter.Add(r, ms, h);
}

bool Application::AddHandler(HandlerMethods ms, router_regex r, Handler h)
{
    if (IsRuning())
    {
        return false;
    }

    r.s = join_app_path(_root_url, r.s);
    return _regexRouter.Add(r, ms, h);
}

Handler& Application::find_handler(Context const& ctx)
{
    HandlerMethod m = from_http_method(static_cast<http_method>(ctx->Req().GetMethod()));

    if (m == HandlerMethod::UNSUPPORT)
    {
        return theEmptyHandler;
    }

    std::string const& url = ctx->Req().GetUrl().path;// full;

    RouterResult rr = _equalRouter.Search(url, m);

    if (rr.handler)
    {
        return *rr.handler;
    }

    rr = _startwithsRouter.Search(url, m);

    if (rr.handler)
    {
        return *rr.handler;
    }

    RegexRouterResult rrr = _regexRouter.Search(url, m);

    if (rrr.handler)
    {
        ctx->InitRequestPathParameters(rrr.parameters, rrr.values);
        return *rrr.handler;
    }

    return theEmptyHandler;
}

void Application::Handle(Context ctx)
{
    do_handle(ctx);
}

void Application::do_handle(Context& ctx)
{
    Handler& h = find_handler(ctx);

    if (!h)
    {
        ctx->RenderNofound();
        ctx->Stop();
        return;
    }

    h(ctx);
}

} //namespace da4qi4
