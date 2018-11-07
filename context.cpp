#include "context.hpp"

#include "connection.hpp"
#include "application.hpp"

namespace da4qi4
{

Json theEmptyJson;

Context ContextIMP::Make(ConnectionPtr cnt)
{
    return std::shared_ptr<ContextIMP>(new ContextIMP(cnt));
}

ContextIMP::ContextIMP(ConnectionPtr cnt)
    : _cnt(cnt), _env(cnt->GetApplication().GetTemplateRoot().native())
{
    _env.set_element_notation(inja::ElementNotation::Dot);
    regist_template_enginer_common_functions();
}

ContextIMP::~ContextIMP()
{
}

void ContextIMP::regist_template_enginer_common_functions()
{
    _env.add_callback("_PARAMETER_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->parameter(name);
                }
            }

            return Utilities::theEmptyString;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return Utilities::theEmptyString;
    });


    _env.add_callback("_IS_PARAMETER_EXISTS_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->is_exists_parameter(name);
                }
            }

            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return false;
    });

    _env.add_callback("_HEADER_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->header(name);
                }
            }

            return Utilities::theEmptyString;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return Utilities::theEmptyString;
    });


    _env.add_callback("_IS_HEADER_EXISTS_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->is_exists_header(name);
                }
            }

            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return false;
    });

    _env.add_callback("_URL_PARAMETER_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->url_parameter(name);
                }
            }

            return Utilities::theEmptyString;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return Utilities::theEmptyString;
    });


    _env.add_callback("_IS_URL_PARAMETER_EXISTS_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->is_exists_url_parameter(name);
                }
            }

            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return false;
    });

    _env.add_callback("_PATH_PARAMETER_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->path_parameter(name);
                }
            }

            return Utilities::theEmptyString;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return Utilities::theEmptyString;
    });


    _env.add_callback("_IS_PATH_PARAMETER_EXISTS_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->is_exists_path_parameter(name);
                }
            }

            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return false;
    });

    _env.add_callback("_FORM_DATA_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->form_data(name);
                }
            }

            return Utilities::theEmptyString;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return Utilities::theEmptyString;
    });


    _env.add_callback("_IS_FORM_DATA_EXISTS_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->is_exists_form_data(name);
                }
            }

            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return false;
    });

    _env.add_callback("_COOKIE_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> std::string
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->cookie(name);
                }
            }

            return Utilities::theEmptyString;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return Utilities::theEmptyString;
    });


    _env.add_callback("_IS_COOKIE_EXISTS_", 1
                      , [this](inja::Parsed::Arguments args, Json data) -> bool
    {
        try
        {
            if (args.size() == 1)
            {
                std::string name = _env.get_argument<std::string>(args, 0, data);

                if (!name.empty())
                {
                    return this->is_exists_cookie(name);
                }
            }

            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << e.what()
                      << std::endl;
        }

        return false;
    });
}

Request const& ContextIMP::Req() const
{
    return _cnt->GetRequest();
}

Response& ContextIMP::Res()
{
    return _cnt->GetResponse();
}

Application& ContextIMP::App()
{
    return _cnt->GetApplication();
}

boost::asio::io_context& ContextIMP::IOContext()
{
    return _cnt->GetSocket().get_executor().context();
}

void ContextIMP::InitRequestPathParameters(std::vector<std::string> const& names
                                           , std::vector<std::string> const& values)
{
    _cnt->GetRequest().InitPathParameters(names, values);
}

std::string ContextIMP::render_on_template(Template const& templ, Json const& data
                                           , bool& server_render_error
                                           , std::string& error_detail)
{
    server_render_error = false;
    error_detail.clear();

    try
    {
        return _env.render_template(templ, data);
    }
    catch (std::exception const& e)
    {
        server_render_error = true;
        error_detail = e.what();
    }

    return Utilities::theEmptyString;
}

void ContextIMP::render_on_template(Template const& templ, Json const& data, http_status status)
{
    bool error = false;
    std::string error_detail;
    std::string view = render_on_template(templ, data, error, error_detail);

    Res().SetStatusCode(status);

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

void ContextIMP::Render(http_status status, Json const& data)
{
    std::string template_name = std::to_string(static_cast<int>(status));

    if (auto templ = App().GetTemplates().Get(template_name))
    {
        render_on_template(*templ, data, status);
    }
    else
    {
        Res().ReplyStatus(status);
    }
}

void ContextIMP::Render(std::string const& template_name, Json const& data)
{
    auto templ = App().GetTemplates().Get(template_name);

    if (!templ)
    {
        RenderNofound();
        return;
    }

    render_on_template(*templ, data, HTTP_STATUS_OK);
}

void ContextIMP::Render(Json const& data)
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
        render_on_template(*templ, data, HTTP_STATUS_OK);
        return;
    }

    size_t len = template_name.size();

    if (len > 1 && template_name[len - 1] == '/')
    {
        template_name += "index";
        templ = App().GetTemplates().Get(template_name);

        if (!templ)
        {
            RenderNofound();
            return;
        }

        render_on_template(*templ, data, HTTP_STATUS_OK);
    }
}

void ContextIMP::Bye()
{
    if (!Res().IsChunked())
    {
        _cnt->StartWrite();
    }
    else
    {
        end_chunked_response();
    }
}

void ContextIMP::StartChunkedResponse()
{
    Res().MarkChunked();
    _cnt->StartChunkedWrite();
}

void ContextIMP::ContinueChunkedResponse(std::string const& body)
{
    Res().PushChunkedBody(body, false);
}

void ContextIMP::end_chunked_response()
{
    Res().PushChunkedBody(Utilities::theEmptyString, true);
}


} //namespace da4qi4
