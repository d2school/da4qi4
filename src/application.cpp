#include "daqi/application.hpp"

#include <iostream>

namespace da4qi4
{

static char const* default_app_name = "DAQI-APP";
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

void ApplicationMgr::CreateDefault(const std::string& app_name)
{
    auto app = Application::Default((!app_name.empty() ? app_name
                                     : std::string(default_app_name)));

    _map.insert(std::make_pair(app->GetUrlRoot(), app));
}

bool ApplicationMgr::MountApplication(ApplicationPtr app)
{
    if (_mounted)
    {
        return false;
    }

    assert(!app->GetName().empty());

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
    return (strategy == always_save)
            || ((strategy == size_greater_than) && (filesize_kb > size_base_kb))
            || ((strategy == size_lesser_than) && (filesize_kb < size_base_kb))
            || ((strategy == extension_is) && (extensions.find(extension) != extensions.cend()))
            || ((strategy == extension_is_not) && (extensions.find(extension) == extensions.cend()));
}


Application::Application()
    : _name(default_app_name)
    , _default_charset("utf-8")
    , _root_url("/")
{
    default_init();
}

Application::Application(std::string const& name)
    : _name(name)
    , _default_charset("utf-8")
    , _root_url("/")
    , _template_ext(get_daqi_HTML_template_ext())
{
    default_init();
}

Application::Application(std::string const& name
                         , std::string const& root_url
                         , fs::path const& root_log
                         , fs::path const& root_static
                         , fs::path const& root_template
                         , fs::path const& root_upload
                         , fs::path const& root_temporary
                         , std::string const& template_ext)
    : _name(name)
    , _root_url(root_url)
    , _root_log(root_log)
    , _root_static(root_static)
    , _root_template(root_template)
    , _root_upload(root_upload)
    , _root_temporary(root_temporary)
    , _template_ext(template_ext)
    , _templates(root_template.native(), root_url, template_ext)

{
    this->InitPathes();
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

void Application::default_init()
{
    assert(!IsRuning());

    default_init_pathes();
    default_init_logger();
    default_init_templates();
}

void Application::default_init_logger()
{
    if (_logger == nullptr)
    {
        _logger = log::Null();
    }
}

void Application::default_init_pathes()
{
    _root_log = fs::current_path();
    _root_template = fs::current_path();
    _root_static = fs::current_path();

    _root_upload = fs::temp_directory_path();
    _root_temporary = fs::temp_directory_path();
}

void Application::default_init_templates()
{
    _templates.InitPathes(_root_template.native(), "/", get_daqi_HTML_template_ext());
}

bool Application::InitLogger(std::string const& log_root, log::Level level, size_t max_log_file_size_kb,
                             size_t max_log_file_count)
{
    if (_root_log != log_root)
    {
        _root_log = log_root;
    }

    return InitLogger(level, max_log_file_size_kb, max_log_file_count);
}

bool Application::InitLogger(log::Level level, size_t max_log_file_size_kb, size_t max_log_file_count)
{
    assert(!IsRuning());
    assert(log::IsNull(_logger));

    if (!log::IsNull(_logger))
    {
        return false;
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

    _logger = log::CreateAppLogger(_name, _root_log.native()
                                   , level, max_log_file_size_kb, max_log_file_count);

    if (!_logger)
    {
        std::cerr << "Create appliction" << _name << " logger fail." << std::endl;
        return false;
    }

    return true;
}

bool Application::InitPathes()
{
    assert(!IsRuning());

    std::stringstream ss;

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
        ss << "Init pathes filesystem-exception. " << e.what();
    }
    catch (std::exception const& e)
    {
        ss << "Init pathes exception. " << e.what();
    }
    catch (...)
    {
        ss << "Init pathes unknown exception.";
    }

    if (log::IsNull(_logger))
    {
        std::cerr << ss.str() << std::endl;
    }
    else
    {
        _logger->error(ss.str());
    }

    return false;
}

void Application::UndesiredTemplates()
{
    _templates.Disable();
}

bool Application::InitTemplates(std::string const& template_ext)
{
    assert(!IsRuning());
    assert(_logger != nullptr);

#ifdef NDEBUG

    if (IsAbortive())
    {
        return true;
    }

#endif

    if (!template_ext.empty())
    {
        _template_ext = template_ext;
    }

    if (!_template_ext.empty())
    {
        _templates.InitPathes(_root_template.native(), _root_url, _template_ext);
        return _templates.Preload(_logger);
    }

    return true;
}

bool Application::AddIntercepter(Intercepter::Handler intercepter)
{
    if (IsRuning())
    {
        _logger->warn("Add intercepter fail. application is running.");
        return false;
    }

    _intercepters.push_back(intercepter);
    return true;
}

bool Application::AddHandler(HandlerMethod m, router_equals r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add equals-router {} fail. application is running.", r.s);
        return false;
    }

    r.s = JoinUrlPath(_root_url, r.s);

    std::string error;

    if (!_equalRouter.Add(r, m, h, t, error))
    {
        _logger->error("Add equals-router {} fail. {}.", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethod m, router_starts r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add starts-router {} fail. application is running.", r.s);
        return false;
    }

    r.s = JoinUrlPath(_root_url, r.s);

    std::string error;

    if (!_startwithsRouter.Add(r, m, h, t, error))
    {
        _logger->error("Add starts-router {} fail. {}.", r.s, t, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethod m, router_regex r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add regex-router {} fail. application is running.", r.s);
        return false;
    }

    r.s = JoinUrlPath(_root_url, r.s);

    std::string error;

    if (!_regexRouter.Add(r, m, h, t, error))
    {
        _logger->error("Add regex-router {} fail. {}.", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_equals r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add euqals-router {} fail. application is running.", r.s);
        return false;
    }

    r.s = JoinUrlPath(_root_url, r.s);

    std::string error;

    if (!_equalRouter.Add(r, ms, h, t, error))
    {
        _logger->error("Add equals-router {} fail. {}.", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_starts r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add starts-router {} fail. application is running.", r.s);
        return false;
    }

    r.s = JoinUrlPath(_root_url, r.s);

    std::string error;

    if (!_startwithsRouter.Add(r, ms, h, t, error))
    {
        _logger->error("Add starts router {} fail. {}.", r.s, error);
        return false;
    }

    return true;
}

bool Application::AddHandler(HandlerMethods ms, router_regex r, Handler h, std::string const& t)
{
    if (IsRuning())
    {
        _logger->warn("Add regex-router {} fail. application is running.", r.s);
        return false;
    }

    r.s = JoinUrlPath(_root_url, r.s);

    std::string error;

    if (!_regexRouter.Add(r, ms, h, t, error))
    {
        _logger->error("Add regex-router {} fail. {}.", r.s, error);
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
    HandlerMethod m = from_http_method(ctx->Req().GetMethod());

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
        _logger->warn("Match equals-router for {} exception. {}.", url, rr.error);
    }

    if (rr.handler)
    {
        ctx->SetTemplateName(rr.template_name);
        return rr.handler;
    }

    StartsRouterResult srr = _startwithsRouter.Search(url, m, url_exists);

    if (!srr.error.empty())
    {
        _logger->warn("Match starts-router for {} exception. {}.", url, srr.error);
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
        _logger->warn("Match regex-router for {} exception. {}.", url, rrr.error);
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

bool Application::RegistWebSocket(std::string const& url, UrlFlag urlflag
                                  , Websocket::EventHandlersFactory factory)
{
    if (IsRuning())
    {
        _logger->warn("Regist websocket-handlers-factory on {} fail. application is running.", url);
        return false;
    }

    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);
    _websocket_handlers_factory.insert(std::make_pair(fullpath, std::move(factory)));

    return true;
}

bool Application::IsWebSocketRegistered(std::string const& url, UrlFlag urlflag) const
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);
    return _websocket_handlers_factory.find(fullpath) != _websocket_handlers_factory.cend();
}

std::unique_ptr<Websocket::EventsHandler> Application::CreateWebSocketHandler(std::string const& url, UrlFlag urlflag)
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);
    auto it = _websocket_handlers_factory.find(fullpath);
    return (it == _websocket_handlers_factory.end()) ? nullptr
           : std::unique_ptr<Websocket::EventsHandler>(it->second());
}

void Application::AddWebSocketConnection(Websocket::Connection::Ptr connection)
{
    assert(connection != nullptr);
    assert(!connection->GetID().empty());

    auto full_url_path = connection->GetURLPath();
    assert(!full_url_path.empty());

    {
        std::scoped_lock<std::mutex> lock(_m_4_websocket_connections);
        auto it = _websocket_connections.find(full_url_path);

        if (it != _websocket_connections.end())
        {
            it->second.Add(connection);
        }
        else
        {
            Websocket::Connections storage(connection);
            _websocket_connections.emplace(std::move(full_url_path), std::move(storage));
        }
    }
}

bool Application::RemoveWebSocketConnection(std::string const& url, UrlFlag urlflag, std::string const& id)
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);

    std::scoped_lock<std::mutex> lock(_m_4_websocket_connections);

    auto it = _websocket_connections.find(fullpath);

    if (it == _websocket_connections.end())
    {
        return false;
    }

    return it->second.Remove(id);
}

bool Application::RenameWebSocketConnectionID(std::string const& url, UrlFlag urlflag
                                              , std::string const& old_id, std::string const& new_id)
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);

    std::scoped_lock<std::mutex> lock(_m_4_websocket_connections);

    auto it = _websocket_connections.find(fullpath);

    if (it == _websocket_connections.end())
    {
        return false;
    }

    return it->second.RenameID(old_id, new_id);
}

Websocket::Connection::Ptr Application::WebsocketConnection(std::string const& url, UrlFlag urlflag
                                                            , std::string const& id)
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);

    std::scoped_lock<std::mutex> lock(_m_4_websocket_connections);

    auto it = _websocket_connections.find(fullpath);

    if (it == _websocket_connections.end())
    {
        return nullptr;
    }

    return it->second.Get(id);
}

std::list<Websocket::Connection::Ptr> Application::AllWebSocketConnections(std::string const& url, UrlFlag urlflag)
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);

    std::scoped_lock<std::mutex> lock(_m_4_websocket_connections);

    auto it = _websocket_connections.find(fullpath);

    return (it == _websocket_connections.end()) ? std::list<Websocket::Connection::Ptr>() : it->second.All();
}

std::list<std::string> Application::AllWebSocketConnectionsID(std::string const& url, UrlFlag urlflag)
{
    auto fullpath = MakesureFullUrlPath(url, urlflag, _root_url);

    std::scoped_lock<std::mutex> lock(_m_4_websocket_connections);

    auto it = _websocket_connections.find(fullpath);

    return (it == _websocket_connections.end()) ? std::list<std::string>() : it->second.AllID();
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
