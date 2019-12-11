#ifndef DAQI_SERVER_HPP
#define DAQI_SERVER_HPP

#include <atomic>
#include <list>
#include <string>
#include <functional>

#include <boost/asio/deadline_timer.hpp>

#include "daqi/def/asio_def.hpp"

#include "daqi/server_engine.hpp"
#include "daqi/application.hpp"
#include "daqi/handler.hpp"

namespace da4qi4
{

extern int const _detect_templates_interval_seconds_;

class Server
{
public:
    enum class WithSSL {no, yes};

    using OnNeedSSLPassword = std::function <std::string(std::size_t max_length
                                                         , SSLContextBase::password_purpose purpose)>;
    struct SSLOptions
    {
        explicit SSLOptions()
            : options(boost::asio::ssl::context::default_workarounds
                      | boost::asio::ssl::context::no_sslv2
                      | boost::asio::ssl::context::no_sslv3
                      | boost::asio::ssl::context::no_tlsv1)
            , certificate_file_with_encryption(CertificateWithEncryption::no)
            , private_key_file_format(SSLContextBase::pem), will_verify_client(false)
            , on_need_password(nullptr)
        {
        }

        explicit SSLOptions(OnNeedSSLPassword&& password_callback)
            : options(boost::asio::ssl::context::default_workarounds
                      | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::no_sslv3)
            , certificate_file_with_encryption(CertificateWithEncryption::no)
            , private_key_file_format(SSLContextBase::pem), will_verify_client(false)
            , on_need_password(std::forward<OnNeedSSLPassword>(password_callback))
        {
        }

        SSLOptions(SSLContextBase::options options, OnNeedSSLPassword&& password_callback)
            : options(options)
            , private_key_file_format(SSLContextBase::pem)
            , on_need_password(std::forward<OnNeedSSLPassword>(password_callback))
        {
        }

        void EnableSSLV3()
        {
            options &= ~boost::asio::ssl::context::no_sslv3;
        }

        void EnableTLSV1()
        {
            options &= ~boost::asio::ssl::context::no_tlsv1;
        }

        void EnableVerifyClient()
        {
            will_verify_client = true;
        }

        void DisableVerifyClient()
        {
            will_verify_client = false;
        }

        enum class CertificateWithEncryption {no, yes};

        void InitFiles(std::string certificate_chain_file
                       , std::string private_key_file
                       , std::string tmp_dh_file = ""
                       , SSLContextBase::file_format private_key_file_format = SSLContextBase::pem
                       , CertificateWithEncryption certificate_file_with_encryption = CertificateWithEncryption::no)
        {
            this->certificate_chain_file = std::move(certificate_chain_file);   //.pem
            this->private_key_file = std::move(private_key_file);               //.key
            this->private_key_file_format = private_key_file_format;

            this->tmp_dh_file = std::move(tmp_dh_file);

            if (!tmp_dh_file.empty())
            {
                options |= SSLContextBase::single_dh_use;
            }

            this->certificate_file_with_encryption = certificate_file_with_encryption;
        }

        SSLContextBase::options options;

        std::string certificate_chain_file;

        CertificateWithEncryption certificate_file_with_encryption;

        std::string private_key_file;
        std::string tmp_dh_file;

        SSLContextBase::file_format private_key_file_format;
        bool will_verify_client;

        OnNeedSSLPassword on_need_password;
    };

private:
    Server(Tcp::endpoint endpoint, size_t thread_count, SSLOptions const* ssl_opts = nullptr);

public:
    using IdleFunction = std::function<void (void)>;

    using Ptr = std::unique_ptr<Server>;

    static Ptr Supply(unsigned short port, size_t thread_count);
    static Ptr Supply(std::string const& host, unsigned short port, size_t thread_count);

    static Ptr Supply(std::string const& host, unsigned short port);
    static Ptr Supply(unsigned short port = 80);


    static Ptr SupplyWithSSL(SSLOptions const& ssl_opt, unsigned short port, size_t thread_count);
    static Ptr SupplyWithSSL(SSLOptions const& ssl_opt
                             , std::string const& host, unsigned short port, size_t thread_count);

    static Ptr SupplyWithSSL(SSLOptions const& ssl_opt, std::string const& host, unsigned short port);
    static Ptr SupplyWithSSL(SSLOptions const& ssl_opt, unsigned short port = 443);

    ~Server();

    IOContextPool* GetIOContextPool()
    {
        return &_ioc_pool;
    }

public:
    bool IsWithSSL() const
    {
        return _withssl == WithSSL::yes;
    }
public:
    void Run();
    void Stop();

public:
    void PauseIdleTimer();
    void ResumeIdleTimer();

    bool IsIdleTimerRunning() const
    {
        return _idle_running;
    }

    void EnableDetectTemplates(int interval_seconds = _detect_templates_interval_seconds_)
    {
        assert(interval_seconds > 0);

        _detect_templates_status.interval_seconds = interval_seconds;
        _detect_templates_status.next_timepoint = std::time(nullptr) + interval_seconds;

        _detect_templates = true;

        if (_running && !_idle_running)
        {
            ResumeIdleTimer();
        }
    }

    void DisableDetectTemplates()
    {
        _detect_templates = false;
    }

    bool IsEnabledDetetTemplates() const
    {
        return _detect_templates;
    }

    void AppendIdleFunction(int interval_seconds, IdleFunction func);

public:
    ApplicationPtr DefaultApp(std::string const& name = "");

    bool Mount(ApplicationPtr app);

public:
    ApplicationPtr AddHandler(HandlerMethod m, std::string const& url, Handler h)
    {
        return AddHandler(m, router_equals(url), h);
    }

    ApplicationPtr AddHandler(HandlerMethod m, router_equals r, Handler h);
    ApplicationPtr AddHandler(HandlerMethod m, router_starts r, Handler h);
    ApplicationPtr AddHandler(HandlerMethod m, router_regex r, Handler h);

    ApplicationPtr AddHandler(HandlerMethods ms, std::string const& url, Handler h)
    {
        return AddHandler(ms, router_equals(url), h);
    }
    ApplicationPtr AddHandler(HandlerMethods ms, router_equals r, Handler h);
    ApplicationPtr AddHandler(HandlerMethods ms, router_starts r, Handler h);
    ApplicationPtr AddHandler(HandlerMethods ms, router_regex r, Handler h);

    bool AddEqualsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h);
    bool AddStartsRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h);
    bool AddRegexRouter(HandlerMethod m, std::vector<std::string> const& urls, Handler h);

public:
    ApplicationPtr PrepareApp(std::string const& url);

private:
    void start_accept();
    void do_accept();
    void do_stop();

private:
    void make_default_app_if_empty();
    void make_default_app(std::string const& name);

    void start_idle_timer();
    void on_idle_timer(errorcode const& ec);
    void stop_idle_timer();

private:
    WithSSL _withssl;
    int _idle_interval_seconds;

private:
    std::atomic_bool _running;
    std::atomic_bool _stopping;

    IOContextPool _ioc_pool;
    Tcp::acceptor _acceptor;
    boost::asio::signal_set _signals;
    std::unique_ptr<boost::asio::ssl::context> _ssl_ctx;

    std::atomic_bool _idle_running;

    std::atomic_bool _detect_templates;
    boost::asio::deadline_timer _idle_timer;

    struct IdleFunctionStatus
    {
        IdleFunctionStatus()
            : interval_seconds(0), next_timepoint(static_cast<std::time_t>(0))
        {}

        IdleFunctionStatus(IdleFunctionStatus const&) = default;

        IdleFunctionStatus(int interval_seconds, IdleFunction func)
            : interval_seconds(interval_seconds),
              next_timepoint(std::time(nullptr) + interval_seconds), func(func)
        {}

        int interval_seconds;
        std::time_t next_timepoint;
        IdleFunction func;
    };

    IdleFunctionStatus _detect_templates_status;
    std::list<IdleFunctionStatus> _idle_functions;

private:
    static int call_idle_function_if_timeout(std::time_t now, IdleFunctionStatus& status);
};

}

#endif // DAQI_SERVER_HPP
