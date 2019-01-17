#include "daqi/context.hpp"

#include <iterator>

#include "daqi/def/log_def.hpp"
#include "daqi/connection.hpp"
#include "daqi/application.hpp"

namespace da4qi4
{

std::string ContextIMP::session_data_name = "_session_data_";
std::string ContextIMP::model_data_name = "_model_data_";

RedisClientPtr init_redis_client(ConnectionPtr cnt)
{
    size_t index = cnt->GetIOContextIndex();
    return RedisPool().Get(index);
}

Context ContextIMP::Make(ConnectionPtr cnt)
{
    return std::shared_ptr<ContextIMP>(new ContextIMP(cnt));
}

ContextIMP::ContextIMP(ConnectionPtr cnt)
    : _cnt(cnt)
    , _env(cnt->HasApplication() ? cnt->GetApplication()->GetTemplateRoot().native() : "")
    , _redis(init_redis_client(cnt))
{
    init_template_env(_env);
    regist_template_enginer_common_functions();
}

ContextIMP::~ContextIMP()
{
}

size_t ContextIMP::IOContextIndex() const
{
    return _cnt->GetIOContextIndex();
}

log::LoggerPtr ContextIMP::logger()
{
    assert(_cnt && _cnt->GetApplication() && _cnt->GetApplication()->GetLogger());
    return _cnt->GetApplication()->GetLogger();
}

void ContextIMP::RegistStringFunctionWithOneStringParameter(char const* function_name,
                                                            PSSFun func,
                                                            std::string defaultValue)
{
    _env.add_callback(function_name, 1
                      , [this, func, function_name, defaultValue](inja::Arguments & args) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = args.at(0)->get<std::string>();

                if (!name.empty())
                {
                    return (this->*func)(name);
                }
            }

            return defaultValue;
        }
        catch (std::exception const& e)
        {
            logger()
            ->error("Regist templage enginer function {} exception. {}", function_name, e.what());
        }

        return defaultValue;
    });
}

void ContextIMP::RegistBoolFunctionWithOneStringParameter(char const* function_name,
                                                          PBSFun func,
                                                          bool defaultValue)
{
    _env.add_callback(function_name, 1
                      , [this, func, function_name, defaultValue](inja::Arguments & args) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = args.at(0)->get<std::string>();

                if (!name.empty())
                {
                    return (this->*func)(name);
                }
            }

            return defaultValue;
        }
        catch (std::exception const& e)
        {
            logger()
            ->error("Regist templage enginer function {} exception. {}"
                    , function_name, e.what());
        }

        return defaultValue;
    });
}

void ContextIMP::regist_template_enginer_common_functions()
{
    RegistStringFunctionWithOneStringParameter("_PARAMETER_", &Self::parameter);
    RegistBoolFunctionWithOneStringParameter("_IS_EXISTS_PARAMETER_", &Self::is_exists_parameter);

    RegistStringFunctionWithOneStringParameter("_HEADER_", &Self::header);
    RegistBoolFunctionWithOneStringParameter("_IS_EXISTS_HEADER_", &Self::is_exists_header);

    RegistStringFunctionWithOneStringParameter("_URL_PARAMETER_", &Self::url_parameter);
    RegistBoolFunctionWithOneStringParameter("_IS_EXISTS_URL_PARAMETER_", &Self::is_exists_url_parameter);

    RegistStringFunctionWithOneStringParameter("_PATH_PARAMETER_", &Self::path_parameter);
    RegistBoolFunctionWithOneStringParameter("_IS_EXISTS_PATH_PARAMETER_", &Self::is_exists_path_parameter);

    RegistStringFunctionWithOneStringParameter("_FORM_DATA_", &Self::form_data);
    RegistBoolFunctionWithOneStringParameter("_IS_EXISTS_FORM_DATA_", &Self::is_exists_form_data);

    RegistStringFunctionWithOneStringParameter("_COOKIE_", &Self::cookie);
    RegistBoolFunctionWithOneStringParameter("_IS_EXISTS_COOKIE_", &Self::is_exists_cookie);
}

Request const& ContextIMP::Req() const
{
    return _cnt->GetRequest();
}

Response& ContextIMP::Res()
{
    return _cnt->GetResponse();
}

Application&  ContextIMP::App()
{
    assert(_cnt->GetApplication() != nullptr);
    return *(_cnt->GetApplication());
}

Application const& ContextIMP::App() const
{
    assert(_cnt->GetApplication() != nullptr);
    return *(_cnt->GetApplication());
}

IOC& ContextIMP::IOContext()
{
    return _cnt->GetSocket().get_executor().context();
}

void ContextIMP::InitRequestPathParameters(std::vector<std::string> const& names
                                           , std::vector<std::string> const& values)
{
    _cnt->GetRequest().InitPathParameters(names, values);
}

std::string ContextIMP::render_on_template(std::string const& templ_name, Template const& templ
                                           , Json const& data
                                           , bool& server_render_error
                                           , std::string& error_what)
{
    server_render_error = false;
    error_what.clear();

    try
    {
        return _env.render(templ, data);
    }
    catch (std::runtime_error const& e)
    {
        server_render_error = true;

        /* format is :  "[inja.exception." + type + "] " + message */
        std::string exception_type;
        std::string exception_message = e.what();

        static std::string const inja_exception_prefix = "[inja.exception.";
        auto pos = exception_message.find(inja_exception_prefix);
        std::string::size_type offset = 0;

        bool formated = (pos == 0);

        if (formated)
        {
            offset = inja_exception_prefix.size();
            auto type_end = exception_message.find("] ", offset);

            if (type_end == std::string::npos)
            {
                formated = false;
            }
            else
            {
                exception_type = exception_message.substr(offset, type_end - offset);
                exception_message = exception_message.substr(type_end + 2);
                error_what = exception_type + ", " + exception_message;
            }
        }

        if (!formated)
        {
            exception_type = "unknown";
            error_what = exception_message;
        }

        logger()->error("Render template \"{}\" exception. {}. {}"
                        , templ_name, exception_type, exception_message);
    }
    catch (std::exception const& e)
    {
        server_render_error = true;
        error_what = e.what();

        logger()->error("Render template \"{}\" exception. {}"
                        , templ_name, e.what());
    }
    catch (...)
    {
        server_render_error = true;
        error_what = "unknown render error.";

        logger()->error("Render template \"{}\" exception.", templ_name);
    }

    return Utilities::theEmptyString;
}

void ContextIMP::render_on_template(std::string const& templ_name, Template const& templ
                                    , Json const& data, http_status status)
{
    bool error = false;
    std::string error_detail;

    std::string view = render_on_template(templ_name, templ, data, error, error_detail);

    if (error)
    {
        if (status != HTTP_STATUS_INTERNAL_SERVER_ERROR)
        {
            Json error_data;
            error_data["internal_server_error_detail"] = error_detail;
            RenderInternalServerError(error_data);
        }
        else
        {
            Res().ReplyStatus(status);
        }

        return;
    }

    Res().SetStatusCode(status);

    if (!view.empty())
    {
        auto content_type = Res().GetContentType(Response::ContentTypePart::without_chartset);

        if (content_type.empty())
        {
            Res().SetContentType("text/html");
        }

        Res().SetBody(std::move(view));
    }
}

void ContextIMP::Render()
{
    Json& page_data = ModelData();
    (page_data.is_null()) ? this->RenderWithoutData() : this->RenderWithData(page_data);
}

void ContextIMP::RenderWithData(http_status status, Json const& data)
{
    std::string template_name = std::to_string(static_cast<int>(status));

    if (auto templ = App().GetTemplates().Get(template_name))
    {
        render_on_template(template_name, *templ, data, status);
    }
    else
    {
        Res().ReplyStatus(status);
    }
}

void ContextIMP::RenderWithData(std::string const& template_name, Json const& data)
{
    auto templ = App().GetTemplates().Get(template_name);

    if (!templ)
    {
        logger()->error("No found template {0}.", template_name);
        RenderNofound();
        return;
    }

    render_on_template(template_name, *templ, data, HTTP_STATUS_OK);
}

void ContextIMP::RenderWithData(Json const& data)
{
    std::string const& path = Req().GetUrl().path;

    if (path.empty())
    {
        Res().ReplyBadRequest();
        return;
    }

    std::string template_name;

    if (path == "/")
    {
        template_name = "index";
    }
    else if (path[0] == '/')
    {
        template_name = path.substr(1);
    }

    auto templ = App().GetTemplates().Get(template_name);

    if (templ)
    {
        render_on_template(template_name, *templ, data, HTTP_STATUS_OK);
        return;
    }

    bool template_name_is_path_form = !template_name.empty() && (*(--template_name.end()) == '/');

    if (!template_name_is_path_form)
    {
        RenderNofound();
        return;
    }

    template_name += "index";
    templ = App().GetTemplates().Get(template_name);

    if (!templ)
    {
        RenderNofound();
        return;
    }

    render_on_template(template_name, *templ, data, HTTP_STATUS_OK);
}

void ContextIMP::end()
{
    _cnt->StartWrite();
}

void ContextIMP::StartChunkedResponse()
{
    Res().MarkChunked();
}

void ContextIMP::NextChunkedResponse(std::string const& body)
{
    Res().PushChunkedBody(body, false);
}

void ContextIMP::StopChunkedResponse()
{
    Res().PushChunkedBody(Utilities::theEmptyString, true);
}

void ContextIMP::Start()
{
    _intercepter_on = Intercepter::On::Request;

    auto range = App().GetIntercepterChainRange();
    _intercepter_beg = _intercepter_iter = range.first;
    _intercepter_end = range.second;

    do_intercepter_on_req_dir();
}

void ContextIMP::Pass()
{
    next(Intercepter::Result::Pass);
}

void ContextIMP::Stop()
{
    next(Intercepter::Result::Stop);
}

void ContextIMP::do_intercepter_on_req_dir()
{
    assert(_intercepter_on == Intercepter::On::Request);

    if (_intercepter_iter == _intercepter_end)
    {
        _intercepter_on = Intercepter::On::Handle;
        App().Handle(shared_from_this());
        return;
    }

    auto& handler = *_intercepter_iter; //ref!!
    handler(shared_from_this(), Intercepter::On::Request);
}

void ContextIMP::do_intercepter_on_res_dir()
{
    assert(_intercepter_on == Intercepter::On::Response);

    Intercepter::ChainReverseIterator r_intercepter_iter(_intercepter_iter);
    Intercepter::ChainReverseIterator r_end(_intercepter_beg);

    if (r_intercepter_iter == r_end)
    {
        end();
        return;
    }

    auto& handler = *r_intercepter_iter; //ref!!
    handler(shared_from_this(), Intercepter::On::Response);
}

void ContextIMP::next(Intercepter::Result result)
{
    if (_intercepter_on == Intercepter::On::Request)
    {
        next_intercepter_on_req_dir(result);
    }
    else if (_intercepter_on == Intercepter::On::Handle)
    {
        _intercepter_on = Intercepter::On::Response;
        start_intercepter_on_res_dir(result);
    }
    else if (_intercepter_on == Intercepter::On::Response)
    {
        next_intercepter_on_res_dir(result);
    }
}

void ContextIMP::next_intercepter_on_req_dir(Intercepter::Result result)
{
    assert(_intercepter_on == Intercepter::On::Request);

    ++_intercepter_iter;

    switch (result)
    {
        case Intercepter::Result::Pass :
            do_intercepter_on_req_dir();
            break;

        case  Intercepter::Result::Stop :
            _intercepter_on = Intercepter::On::Response;
            do_intercepter_on_res_dir();
            break;
    }
}

void ContextIMP::start_intercepter_on_res_dir(Intercepter::Result result)
{
    switch (result)
    {
        case Intercepter::Result::Pass :
            do_intercepter_on_res_dir();
            break;

        case  Intercepter::Result::Stop :
            end();
            break;
    }
}

void ContextIMP::next_intercepter_on_res_dir(Intercepter::Result result)
{
    assert(_intercepter_on == Intercepter::On::Response);

    --_intercepter_iter;
    start_intercepter_on_res_dir(result);
}

} //namespace da4qi4
