#include "daqi/application.hpp"

#include <iostream>

namespace da4qi4
{

static char const* default_app_name = "da4qi4-Default";

ApplicationMgr& AppMgr()
{
    static ApplicationMgr mgr;
    return mgr;
}

bool ApplicationMgr::CreateDefaultIfEmpty()
{
    if (_map.empty())
    {
        auto app = Application::Default();

        if (!app->Init())
        {
            return false;
        }

        _map.insert(std::make_pair("/", app));
        return true;
    }

    return false;
}

bool ApplicationMgr::MountApplication(ApplicationPtr app)
{
    if (_mounted)
    {
        return false;
    }

    assert(!app->GetName().empty());
    assert(app->GetLogger() != nullptr);

    if (!IsExists(app->GetName()))
    {
        _app_loggers.insert(std::make_pair(app->GetName(), app->GetLogger()));
        _map.insert(std::make_pair(app->GetUrlRoot(), app));
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

ApplicationPtr ApplicationMgr::FindByURL(std::string const& url)
{
    auto l = _map.lower_bound(url);

    if (l == _map.end())
    {
        return nullptr;
    }

    if (Utilities::iStartsWith(url, l->first))
    {
        return l->second;
    }

    auto u = _map.upper_bound(url);

    for (auto it = ++l; it != u && it != _map.end(); ++it)
    {
        if (Utilities::iStartsWith(url, it->first))
        {
            return it->second;
        }
    }

    return nullptr;
}

ApplicationPtr ApplicationMgr::FindByName(std::string const& name)
{
    for (auto a : _map)
    {
        if (a.second->GetName() == name)
        {
            return a.second;
        }
    }

    return nullptr;
}

ApplicationPtr const ApplicationMgr::FindByName(std::string const& name) const
{
    for (auto a : _map)
    {
        if (a.second->GetName() == name)
        {
            return a.second;
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

void ApplicationMgr::Mount()
{
    _mounted = true;

    for (auto a : _map)
    {
        a.second->Mount();
    }
}

void ApplicationMgr::CheckTemplatesUpdate()
{
    for (auto a : _map)
    {
        if (!a.second->GetTemplates().ReloadIfFindUpdate())
        {
            a.second->GetTemplates().ReloadIfFindNew();
        }
    }
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
    , _templates("", "/")
{
}

Application::Application(std::string const& name
                         , std::string const& root_url
                         , fs::path const& root_log
                         , fs::path const& root_static
                         , fs::path const& root_template
                         , fs::path const& root_upload
                         , fs::path const& root_temporary)
    : _name(name)
    , _root_url(root_url)
    , _root_log(root_log)
    , _root_static(root_static)
    , _root_template(root_template)
    , _root_upload(root_upload)
    , _root_temporary(root_temporary)
    , _templates(root_template.native(), root_url)

{
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

bool Application::init_logger(NeedLogger will_create_logger, log::Level level,
                              size_t max_file_size_kb,
                              size_t max_file_count)
{
    assert(!IsRuning());

    if (will_create_logger == NeedLogger::no)
    {
        _logger = log::Null();
        return true;
    }

    try
    {
        if (_root_log.empty())
        {
            _root_log = fs::current_path();
        }
        else if (!_root_log.is_absolute())
        {
            _root_log = fs::absolute(_root_log);
        }
    }
    catch (fs::filesystem_error const& e)
    {
        std::cerr << "Init " << _name << " log path exception. " << e.what() << std::endl;
        return false;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Init " << _name << " log path exception. " << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cerr << "Init " << _name << " log path unknown exception." << std::endl;
        return false;
    }

    _logger = log::CreateAppLogger(_name, _root_log.native(), level, max_file_size_kb, max_file_count);

    if (!_logger)
    {
        std::cerr << "Create appliction" << _name << " logger fail." << std::endl;
        return false;
    }

    return true;
}

bool Application::init_pathes()
{
    assert(!IsRuning());

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

        if (_root_static.empty())
        {
            _root_static = fs::current_path();
        }
        else if (!_root_static.is_absolute())
        {
            _root_static = fs::absolute(_root_static);
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

        return true;
    }
    catch (fs::filesystem_error const& e)
    {
        _logger->error("Init pathes exception. {}", e.what());
    }
    catch (std::exception const& e)
    {
        _logger->error("Init pathes exception. {}", e.what());
    }
    catch (...)
    {
        _logger->error("Init pathes unknown exception.");
    }

    return false;
}

bool Application::init_templates()
{
    assert(!IsRuning());
    assert(_logger != nullptr);

    return _templates.Preload(_logger);
}

bool Application::AddHandler(HandlerMethod m, router_equals r, Handler h)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_equalRouter.Add(r, m, h, error))
    {
        _logger->error("Add router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethod m, router_starts r, Handler h)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_startwithsRouter.Add(r, m, h, error))
    {
        _logger->error("Add starts router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethod m, router_regex r, Handler h)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_regexRouter.Add(r, m, h, error))
    {
        _logger->error("Add regex router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_equals r, Handler h)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_equalRouter.Add(r, ms, h, error))
    {
        _logger->error("Add equals router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_starts r, Handler h)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_startwithsRouter.Add(r, ms, h, error))
    {
        _logger->error("Add starts router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_regex r, Handler h)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_regexRouter.Add(r, ms, h, error))
    {
        _logger->error("Add regex router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

Handler& Application::find_handler(Context const& ctx)
{
    HandlerMethod m = from_http_method(static_cast<http_method>(ctx->Req().GetMethod()));

    if (m == HandlerMethod::UNSUPPORT)
    {
        return theEmptyHandler;
    }

    std::string const& url = ctx->Req().GetUrl().path;

    RouterResult rr = _equalRouter.Search(url, m);

    if (!rr.error.empty())
    {
        _logger->warn("Match equals router for {} exception. {}", url, rr.error);
    }

    if (rr.handler)
    {
        return *rr.handler;
    }

    rr = _startwithsRouter.Search(url, m);

    if (!rr.error.empty())
    {
        _logger->warn("Match starts router for {} exception. {}", url, rr.error);
    }

    if (rr.handler)
    {
        return *rr.handler;
    }

    RegexRouterResult rrr = _regexRouter.Search(url, m);

    if (!rrr.error.empty())
    {
        _logger->warn("Match regex router for {} exception. {}", url, rrr.error);
    }

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
        ctx->RenderWithoutData();
        ctx->Pass();
        return;
    }

    h(ctx);
}

} //namespace da4qi4
