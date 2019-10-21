#include "daqi/application.hpp"

#include <iostream>

namespace da4qi4
{

static char const* default_app_name = "da4qi4-Default";
static char const* abortive_app_name = "ABORTIVE-APP";

namespace
{
std::string default_abortive_app_log_root = "./";
std::string abortive_app_log_root = default_abortive_app_log_root;
}

ApplicationPtr Application::Abortive()
{
    ApplicationPtr app = Application::Default();
    app->_root_log = abortive_app_log_root;
    app->_name = abortive_app_name;
#ifdef NDEBUG
    app->_is_abortive = true;
#endif
    app->Init(ActualLogger::yes, log::Level::trace);
    return app;
}

void ApplicationMgr::InitAbortiveAppLogRoot(std::string const& log_path)
{
    if (abortive_app_log_root == default_abortive_app_log_root
        && log_path != default_abortive_app_log_root)
    {
        abortive_app_log_root = log_path;
    }
}

std::string const& ApplicationMgr::GetAbortiveAppLogRoot()
{
    return abortive_app_log_root;
}

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

    if (app->GetLogger() == nullptr)
    {
        app->Init();
    }

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
#ifdef NDEBUG
        return _abortive_app;
#else
        return nullptr;
#endif
    }

    if (Utilities::iStartsWith(url, l->first))
    {
        return l->second;
    }

    for (++l; l != _map.end(); ++l)
    {
        if (Utilities::iStartsWith(url, l->first))
        {
            return l->second;
        }
    }

#ifdef NDEBUG
    return _abortive_app;
#else
    return nullptr;
#endif
}

ApplicationPtr ApplicationMgr::FindByName(std::string const& name)
{
    for (auto& a : _map)
    {
        if (a.second->GetName() == name)
        {
            return a.second;
        }
    }

#ifdef NDEBUG

    if (name == abortive_app_name)
    {
        return _abortive_app;
    }

#endif

    return nullptr;
}

ApplicationPtr const ApplicationMgr::FindByName(std::string const& name) const
{
    for (auto const& a : _map)
    {
        if (a.second->GetName() == name)
        {
            return a.second;
        }
    }

#ifdef NDEBUG

    if (name == abortive_app_name)
    {
        return _abortive_app;
    }

#endif

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

    for (auto& a : _map)
    {
        a.second->Mount();
    }
}

void ApplicationMgr::CheckTemplatesUpdate()
{
    for (auto& a : _map)
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

bool Application::init_logger(ActualLogger will_create_logger, log::Level level,
                              size_t max_file_size_kb,
                              size_t max_file_count)
{
    assert(!IsRuning());

    if (will_create_logger == ActualLogger::no)
    {
        _logger = log::Null();
        return true;
    }

    try
    {
        if (_root_log.empty())
        {
            _root_log = fs::current_path() / "/";
        }

        if (!_root_log.is_absolute())
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

#ifdef NDEBUG

    if (IsAbortive())
    {
        return true;
    }

#endif

    return _templates.Preload(_logger);
}

bool Application::AddHandler(HandlerMethod m, router_equals r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_equalRouter.Add(r, m, h, t, error))
    {
        _logger->error("Add router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethod m, router_starts r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_startwithsRouter.Add(r, m, h, t, error))
    {
        _logger->error("Add starts router {} fail. {}", r.s, t, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethod m, router_regex r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_regexRouter.Add(r, m, h, t, error))
    {
        _logger->error("Add regex router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_equals r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_equalRouter.Add(r, ms, h, t, error))
    {
        _logger->error("Add equals router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_starts r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_startwithsRouter.Add(r, ms, h, t, error))
    {
        _logger->error("Add starts router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_regex r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add router {} fail. application is running.");
        return false;
    }

    r.s = join_app_path(_root_url, r.s);

    std::string error;

    if (!_regexRouter.Add(r, ms, h, t, error))
    {
        _logger->error("Add regex router {} fail. {}", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddEqualsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h,
                                  std::string const& t)
{
    for (auto a : urls)
    {
        if (!AddHandler(m, router_equals(a), h, t))
        {
            return false;
        }
    }

    return true;
}

bool Application::AddStartsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h,
                                  std::string const& t)
{
    for (auto a : urls)
    {
        if (!AddHandler(m, router_starts(a), h, t))
        {
            return false;
        }
    }

    return true;
}

bool Application::AddRegexRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h,
                                 std::string const& t)
{
    for (auto a : urls)
    {
        if (!AddHandler(m, router_regex(a), h, t))
        {
            return false;
        }
    }

    return true;
}

std::vector<UniformItem> Application::GetEqualsRouterUniformItems() const
{
    return _equalRouter.Uniforms();
}

std::vector<UniformItem> Application::GetStartsRouterUniformItems() const
{
    return _startwithsRouter.Uniforms();
}

std::vector<UniformRegexItem> Application::GetRegexRouterUniformItems() const
{
    return _regexRouter.Uniforms();
}

Handler* Application::find_handler(Context const& ctx, bool& url_exists, bool& unsupport_method
                                   , std::string const& retry_path)
{
    HandlerMethod m = from_http_method(static_cast<http_method>(ctx->Req().GetMethod()));

    if (m == HandlerMethod::UNSUPPORT)
    {
        unsupport_method = true;
        ctx->ClearTemplateName();
        return nullptr;
    }

    std::string const& url = (retry_path.empty() ? ctx->Req().GetUrl().path : retry_path);

    RouterResult rr = _equalRouter.Search(url, m, url_exists);

    if (!rr.error.empty())
    {
        _logger->warn("Match equals router for {} exception. {}", url, rr.error);
    }

    if (rr.handler)
    {
        ctx->SetTemplateName(rr.template_name);
        return rr.handler;
    }

    StartsRouterResult srr = _startwithsRouter.Search(url, m, url_exists);

    if (!srr.error.empty())
    {
        _logger->warn("Match starts router for {} exception. {}", url, srr.error);
    }

    if (srr.handler)
    {
        ctx->SetTemplateName(srr.template_name);
        auto remain = url.size() - srr.key.size();
        ctx->InitRequestPathParameter(url.substr(srr.key.size(), remain));

        return srr.handler;
    }

    RegexRouterResult rrr = _regexRouter.Search(url, m, url_exists);

    if (!rrr.error.empty())
    {
        _logger->warn("Match regex router for {} exception. {}", url, rrr.error);
    }

    if (rrr.handler)
    {
        ctx->SetTemplateName(rrr.template_name);
        ctx->InitRequestPathParameters(rrr.parameters, rrr.values);
        return rrr.handler;
    }

    return nullptr;
}

void Application::Handle(Context ctx)
{
    do_handle(ctx);
}

void Application::do_handle(Context& ctx)
{
    bool url_exists = false;
    bool unsupport_method = false;
    bool retring = false;

    Handler* h = find_handler(ctx, url_exists, unsupport_method);

    while (!h || !*h)
    {
        if (url_exists || unsupport_method)
        {
            ctx->RenderNotImplemented().Pass();
            return;
        }

        if (retring)
        {
            ctx->RenderWithoutData().Pass();
            return;
        }

        retring = true;

        if (Utilities::EndsWith(ctx->Req().GetUrl().path, "/index"))
        {
            const int len_of_str_index = 5; //len of "index"
            std::size_t len = ctx->Req().GetUrl().path.length();
            auto try_path = ctx->Req().GetUrl().path.substr(0, len - len_of_str_index);

            h = find_handler(ctx, url_exists, unsupport_method, try_path);
        }
    }

    try
    {
        (*h)(ctx);
    }
    catch (std::exception const& e)
    {
        _logger->error("Handler {} exception. {}", ctx->Req().GetUrl().full, e.what());
    }
}

log::LoggerPtr ApplicationMgr::GetApplicationLogger(std::string const& application_name)
{
    auto it = _app_loggers.find(application_name);
#ifndef NDEBUG
    return (it == _app_loggers.end() ? log::Null() : it->second);
#else

    return (it != _app_loggers.end()) ? it->second :
           ((application_name == abortive_app_name) ? _abortive_app->GetLogger() : log::Null());
#endif
}

void AddDa4Qi4DefaultHandler(ApplicationPtr app)
{
    app->AddHandler(_GET_, "/.da4qi4", [app](Context ctx)
    {
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n<html>\n<head>\n<title>"
           << app->GetName() << " on da4qi4</title>\n"
           << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
           << "<meta name=\"description\" content=\"default da4qi4-web-routers page\" />\n"
           << "</head>\n<body>";

        {
            ss << "<h1>" << app->GetName() << "</h1>\n";
            ss << "<ul>\n"
               << "<li>url-root : " << app->GetUrlRoot() << "</li>\n"
#ifndef NDEBUG
               << "<li>template-root : " << app->GetTemplateRoot().string() << "</li>\n"
               << "<li>static-root : " << app->GetStaticRootPath().string() << "</li>\n"
               << "<li>upload-root : " << app->GetUploadRoot().string() << "</li>\n"
#endif
               << "<li>max-upload-bytes : " << app->GetUpoadMaxSizeLimitKB() << "KB</li>\n"
               << "<li>default-charset : " << app->GetDefaultCharset() << "</li>\n";
            ss << "</ul>\n";
        }

        {
            auto items = app->GetEqualsRouterUniformItems();
            ss << "<h2>Equals-Routers</h2>\n<ol>\n";

            for (auto const& i : items)
            {
                ss << "<li>" << i.method << "&nbsp;&nbsp; "
                   << i.url_matcher << "&nbsp;&nbsp;"
                   << i.template_name << "</li>\n";
            }

            ss << "</ol>\n";
        }

        {
            auto items = app->GetStartsRouterUniformItems();
            ss << "<h2>Starts-Routers</h2>\n<ol>\n";

            for (auto const& i : items)
            {
                ss << "<li>" << i.method << "&nbsp;&nbsp; "
                   << i.url_matcher << "&nbsp;&nbsp;"
                   << i.template_name << "</li>\n";
            }

            ss << "</ol>\n";
        }

        {
            auto items = app->GetRegexRouterUniformItems();
            ss << "<h2>Regex-Routers</h2>\n<ol>\n";

            for (auto const& i : items)
            {
                ss << "<li>" << i.method << "&nbsp;&nbsp;" << i.url_matcher
                   << "&nbsp;&nbsp;" << i.regex_matcher
                   << "&nbsp;&nbsp;" << i.template_name << "</li>\n";
            }

            ss << "</ol>\n";
        }

        ss << "</body></html>";

        ctx->Res().ReplyOk(ss.str());
        ctx->Pass();
    });
}

} //namespace da4qi4
