#ifndef DAQI_APPLICATION_HPP
#define DAQI_APPLICATION_HPP

#include <set>
#include <map>
#include <memory>

#include "daqi/def/log_def.hpp"
#include "daqi/def/boost_def.hpp"
#include "daqi/utilities/string_utilities.hpp"

#include "daqi/router.hpp"
#include "daqi/templates.hpp"
#include "daqi/intercepter.hpp"

#include "daqi/websocket/handler_websocket.hpp"
#include "daqi/websocket/connection_websocket.hpp"

namespace da4qi4
{

struct UploadFileSaveOptions
{
    enum Strategy
    {
        always_save
        , alway_no_save
        , size_greater_than
        , size_lesser_than
        , extension_is
        , extension_is_not
    };

    bool IsNeedSave(std::string const& extension, size_t filesize_kb) const;

    Strategy strategy = always_save;
    size_t size_base_kb;
    std::set<std::string, Utilities::IgnoreCaseCompare> extensions;
};

struct AppLocalDiskSetting
{
    AppLocalDiskSetting()
        : template_ext(get_daqi_HTML_template_ext())
    {}

    AppLocalDiskSetting(AppLocalDiskSetting const&) = default;
    AppLocalDiskSetting(AppLocalDiskSetting&&) = default;

    std::string log_dir;
    std::string static_dir;

    std::string template_dir;
    std::string template_ext;

    std::string temporary_dir;
    std::string upload_dir;
};

struct AppLoggerSetting
{
    static log::Level const  default_log_level = log::Level::info;
    static std::size_t const default_max_log_file_size_kb = 5 * 1024;
    static std::size_t const default_max_log_file_count = 9;

    AppLoggerSetting()
        : level(default_log_level),
          max_log_file_size_kb(default_max_log_file_size_kb),
          max_log_file_count(default_max_log_file_count)
    {}

    log::Level level;
    size_t max_log_file_size_kb;
    size_t max_log_file_count;
};


class Application;
using ApplicationPtr = std::shared_ptr<Application>;

class Application
{
private:
    Application();
    Application(std::string const& name);

    Application(std::string const& name
                , std::string const& root_url
                , fs::path const& root_log
                , fs::path const& root_static
                , fs::path const& root_template
                , fs::path const& root_upload
                , fs::path const& root_temporary
                , std::string const& template_ext
               );
public:
    static ApplicationPtr Default()
    {
        return ApplicationPtr(new Application());
    }

    static ApplicationPtr Default(std::string const& name)
    {
        return ApplicationPtr(new Application(name));
    }

    static ApplicationPtr Customize(std::string const& name
                                    , std::string const& root_url
                                    , fs::path const& root_log
                                    , fs::path const& root_static = ""
                                    , fs::path const& root_template = ""
                                    , fs::path const& root_upload = ""
                                    , fs::path const& root_temporary = ""
                                    , std::string const& template_ext = get_daqi_HTML_template_ext())
    {
        return ApplicationPtr(new Application(name,
                                              root_url, root_log, root_static,
                                              root_template, root_upload,
                                              root_temporary, template_ext));
    }

    static ApplicationPtr Customize(std::string const& name, std::string const& url
                                    , AppLocalDiskSetting const& ads)
    {
        return Customize(name,
                         url, ads.log_dir, ads.static_dir,
                         ads.template_dir, ads.upload_dir,
                         ads.temporary_dir, ads.template_ext);
    }

    static ApplicationPtr Abortive();

    Application(Application const&) = delete;
    Application& operator()(Application const&) = delete;

    ~Application();

    bool Init(AppLoggerSetting const& logger_setting, std::string const& template_ext = "")
    {
        return Init(logger_setting.level
                    , logger_setting.max_log_file_size_kb, logger_setting.max_log_file_count, template_ext);
    }

    bool Init(log::Level level, std::string const& template_ext)
    {
        return Init(level
                    , AppLoggerSetting::default_max_log_file_size_kb, AppLoggerSetting::default_max_log_file_count
                    , template_ext);
    }

    bool Init(log::Level level = AppLoggerSetting::default_log_level,
              size_t max_log_file_size_kb = AppLoggerSetting::default_max_log_file_size_kb,
              size_t max_log_file_count = AppLoggerSetting::default_max_log_file_count,
              std::string const& template_ext = "")
    {
        return InitLogger(level, max_log_file_size_kb, max_log_file_count)
               && InitPathes()
               && InitTemplates(template_ext);
    }

    bool Init(std::string const& log_root, log::Level level)
    {
        return Init(log_root, level, AppLoggerSetting::default_max_log_file_size_kb
                    , AppLoggerSetting::default_max_log_file_count);
    }

    bool Init(std::string const& log_root, log::Level level = AppLoggerSetting::default_log_level,
              size_t max_log_file_size_kb = AppLoggerSetting::default_max_log_file_size_kb,
              size_t max_log_file_count = AppLoggerSetting::default_max_log_file_count)
    {
        return InitLogger(log_root, level, max_log_file_size_kb, max_log_file_count)
               && InitPathes() && InitTemplates();
    }

    bool InitLogger(std::string const& log_root, AppLoggerSetting const& logger_setting)
    {
        return InitLogger(log_root
                          , logger_setting.level, logger_setting.max_log_file_size_kb
                          , logger_setting.max_log_file_count);
    }

    bool InitLogger(std::string const& log_root,
                    log::Level level = AppLoggerSetting::default_log_level,
                    size_t max_log_file_size_kb = AppLoggerSetting::default_max_log_file_size_kb,
                    size_t max_log_file_count = AppLoggerSetting::default_max_log_file_count);

    bool InitLogger(AppLoggerSetting const& logger_setting)
    {
        return InitLogger(logger_setting.level, logger_setting.max_log_file_size_kb, logger_setting.max_log_file_count);
    }

    bool InitLogger(log::Level level = AppLoggerSetting::default_log_level,
                    size_t max_log_file_size_kb = AppLoggerSetting::default_max_log_file_size_kb,
                    size_t max_log_file_count = AppLoggerSetting::default_max_log_file_count);

    bool InitPathes();
    bool InitTemplates(std::string const& template_ext = "");
    void UndesiredTemplates();

#ifdef NDEBUG
    bool IsAbortive() const
    {
        return _is_abortive;
    }
#endif

    void Mount()
    {
        _mounted = true;
    }

    bool IsRuning() const
    {
        return  _mounted && !_disabled;
    }

    Application& SetStaticRoot(std::string const& root_static)
    {
        _root_static = root_static;
        return *this;
    }

    Application& SetLogRoot(std::string const& root_log)
    {
        if (!IsRuning() && log::IsNull(_logger))
        {
            _root_log = root_log;
        }

        return *this;
    }

    Application& SetTemplateRoot(std::string const& root_template)
    {
        if (!IsRuning())
        {
            _root_template = root_template;
        }

        return *this;
    }

    Application& SetTemplateExt(std::string const& template_ext)
    {
        if (!IsRuning() && _template_ext != template_ext)
        {
            _template_ext = template_ext;
        }

        return *this;
    }

    Application& SetTemporaryRoot(std::string const& root_temporary)
    {
        if (!IsRuning())
        {
            _root_temporary = root_temporary;
        }

        return *this;
    }

    Application& SetDefaultCharset(std::string const& charset)
    {
        if (!IsRuning())
        {
            _default_charset = charset;
        }

        return *this;
    }

    Application& SetUploadRoot(std::string const& root_upload)
    {
        if (!IsRuning())
        {
            _root_upload = root_upload;
        }

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

    fs::path const& GetLogRoot() const
    {
        return _root_log;
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

    std::string const& GetTempateExt() const
    {
        return _template_ext;
    }

    bool IsEnable() const
    {
        return !_disabled;
    }

    void Disable();
    void Enable();

    void CheckUpdate();

    size_t GetUpoadMaxSizeLimitKB() const
    {
        return _upload_max_size_limit_kb;
    }

    void SetUpoadMaxSizeLimitKB(size_t size_limit_kb)
    {
        _upload_max_size_limit_kb = size_limit_kb;
    }

    UploadFileSaveOptions const& GetUploadFileSaveOptions() const
    {
        return _upload_file_save_opt;
    }

    UploadFileSaveOptions& GetUploadFileSaveOptions()
    {
        return _upload_file_save_opt;
    }

    Templates const& GetTemplates() const
    {
        return _templates;
    }

    Templates& GetTemplates()
    {
        return _templates;
    }

    log::LoggerPtr GetLogger()
    {
        return _logger;
    }

    void Handle(Context ctx);

public:
    bool AddHandler(HandlerMethod m, std::string const& url, Handler h, std::string const& t = "")
    {
        return AddHandler(m, router_equals(url), h, t);
    }

    bool AddHandler(HandlerMethod m, router_equals r, Handler h, std::string const& t = "");
    bool AddHandler(HandlerMethod m, router_starts r, Handler h, std::string const& t = "");
    bool AddHandler(HandlerMethod m, router_regex r, Handler h, std::string const& t = "");

    bool AddHandler(HandlerMethods ms, std::string const& url, Handler h, std::string const& t = "")
    {
        return AddHandler(ms, router_equals(url), h, t);
    }
    bool AddHandler(HandlerMethods ms, router_equals e, Handler h, std::string const& t = "");
    bool AddHandler(HandlerMethods ms, router_starts e, Handler h, std::string const& t = "");
    bool AddHandler(HandlerMethods ms, router_regex e, Handler h, std::string const& t = "");

    bool AddEqualsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h
                         , std::string const& t = "");
    bool AddStartsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h
                         , std::string const& t = "");
    bool AddRegexRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h
                        , std::string const& t = "");

public:
    bool AddIntercepter(Intercepter::Handler intercepter);

    std::pair<Intercepter::ChainIterator, Intercepter::ChainIterator>
    GetIntercepterChainRange()
    {
        return { _intercepters.begin(), _intercepters.end() };
    }

public:
    std::vector<UniformItem> GetEqualsRouterUniformItems() const;
    std::vector<UniformItem> GetStartsRouterUniformItems() const;
    std::vector<UniformRegexItem> GetRegexRouterUniformItems() const;

public:
    bool RegistWebSocket(std::string const& url, UrlFlag urlflag, Websocket::EventHandlersFactory factory);
    bool IsWebSocketRegistered(std::string const& url, UrlFlag urlflag) const;

    std::unique_ptr<Websocket::EventsHandler> CreateWebSocketHandler(std::string const& url, UrlFlag urlflag);
public:
    void AddWebSocketConnection(Websocket::Connection::Ptr connection);
    Websocket::Connection::Ptr WebsocketConnection(std::string const& url, UrlFlag urlflag, std::string const& id);
    bool RemoveWebSocketConnection(std::string const& url, UrlFlag urlflag, std::string const& id);
    bool RenameWebSocketConnectionID(std::string const& url, UrlFlag urlflag
                                     , std::string const& old_id, std::string const& new_id);

    std::list<Websocket::Connection::Ptr> AllWebSocketConnections(std::string const& url, UrlFlag urlflag);
    std::list<std::string> AllWebSocketConnectionsID(std::string const& url_full, UrlFlag urlflag);

private:
    void default_init();
    void default_init_pathes();
    void default_init_logger();
    void default_init_templates();

private:
    Handler* find_handler(const Context& ctx, bool& url_exists, bool& unsupport_method
                          , std::string const& retry_path = "");
    void do_handle(Context& ctx);

private:
    EqualsRoutingTable _equalRouter;
    StartsWithRoutingTable _startwithsRouter;
    RegexMatchRoutingTable _regexRouter;

private:
    bool _disabled = false;
    bool _mounted = false;

    std::string _name;
    std::string _default_charset = "UTF-8";

    std::string _root_url;
    fs::path _root_log;
    fs::path _root_static;
    fs::path _root_template;
    fs::path _root_upload;
    fs::path _root_temporary;
    std::string _template_ext;

    size_t _upload_max_size_limit_kb = 1 * 1024 * 1024;

    Templates _templates;

    UploadFileSaveOptions _upload_file_save_opt;

private:
    Intercepter::Chain _intercepters;

private:
    log::LoggerPtr _logger = log::Null();

#ifdef NDEBUG
    bool _is_abortive = false;
#endif

private:
    std::map<std::string /* url-full */, Websocket::EventHandlersFactory> _websocket_handlers_factory;

private:
    std::mutex _m_4_websocket_connections;
    std::map <std::string /* url-full */, Websocket::Connections> _websocket_connections;
};

using ApplicationPtr = std::shared_ptr<Application>;

using ApplicationMap = std::map<std::string /* url */
                       , ApplicationPtr, Utilities::IgnoreCaseCompareDESC>;

class ApplicationMgr
{
public:
    ApplicationMgr() = default;
    ApplicationMgr(ApplicationMgr const&) = delete;
    ApplicationMgr& operator = (ApplicationMgr&) = delete;

    void CreateDefault(std::string const& app_name = "");
    bool MountApplication(ApplicationPtr app);

    void Enable(std::string const& name);
    void Disable(std::string const& name);

    bool IsExists(std::string const& name) const;
    bool IsEnable(std::string const& name) const;

    ApplicationPtr FindByURL(std::string const& url);

    ApplicationPtr FindByName(std::string const& name);
    ApplicationPtr const FindByName(std::string const& name) const;

    log::LoggerPtr GetApplicationLogger(std::string const& application_name);

    ApplicationMap const& All() const
    {
        return _map;
    }

    void Mount();
    void CheckTemplatesUpdate();

    bool IsEmpty() const
    {
        return _map.empty();
    }
    size_t Count() const
    {
        return _map.size();
    }

    bool IsFailedApp(ApplicationPtr ptr)
    {
#ifdef NDEBUG
        return _abortive_app == ptr;
#else
        return false;
#endif
    }

    static void InitAbortiveAppLogRoot(std::string const& log_path);
    static std::string const& GetAbortiveAppLogRoot();

private:
#ifdef NDEBUG
    ApplicationPtr _abortive_app = Application::Abortive();
#endif

    ApplicationMap _map;
    bool _mounted = false;

    using ApplicationLoggerMap = std::map<std::string /* name */, log::LoggerPtr>;

    ApplicationLoggerMap _app_loggers;
};

ApplicationMgr& AppMgr();

void AddDa4Qi4DefaultHandler(ApplicationPtr app);

} // namespace da4qi4

#endif // DAQI_APPLICATION_HPP
