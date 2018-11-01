#ifndef DAQI_CONTEXT_HPP
#define DAQI_CONTEXT_HPP

#include <memory>
#include <functional>
#include <list>

#include "request.hpp"
#include "response.hpp"
#include "template_library.hpp"

#include "nlohmann/json_fwd.hpp"

namespace da4qi4
{
using Json = nlohmann::json;

extern Json theEmptyJson;

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class ContextIMP;
using Context = std::shared_ptr<ContextIMP>;

class Application;

class ContextIMP
{
    ContextIMP(ConnectionPtr cnt);
public:
    static Context Make(ConnectionPtr cnt);
    ~ContextIMP();

    ContextIMP(ContextIMP const&) = delete;
    ContextIMP& operator = (ContextIMP const&) = delete;

    Request const& Req() const;
    Response& Res();

    std::string const& Req(std::string const& name) const
    {
        return Req().GetParameter(name);
    }

    Application& App();

public:
    void Render(http_status status, Json const& data);
    void Render(std::string const& template_name, Json const& data);

public:
    void RenderNofound(Json const& data)
    {
        Render(HTTP_STATUS_NOT_FOUND, data);
    }
    void RenderUnauthorized(Json const& data)
    {
        Render(HTTP_STATUS_UNAUTHORIZED, data);
    }

    void RenderForbidden(Json const& data)
    {
        Render(HTTP_STATUS_FORBIDDEN, data);
    }

    void RenderNotImplemented(Json const& data)
    {
        Render(HTTP_STATUS_NOT_IMPLEMENTED, data);
    }

    void RenderServiceUnavailable(Json const& data)
    {
        Render(HTTP_STATUS_SERVICE_UNAVAILABLE, data);
    }

    void RenderInternalServerError(Json const& data)
    {
        Render(HTTP_STATUS_INTERNAL_SERVER_ERROR, data);
    }

public:
    void Bye();

private:
    std::string render_on_template(inja::Template const& templ, Json const& data
                                   , bool& server_render_error
                                   , std::string& error_detail);

    void regist_template_enginer_common_functions();

    std::string const& parameter(std::string const& name) const
    {
        return this->Req().GetParameter(name);
    }

    bool is_exists_parameter(std::string const& name) const
    {
        return this->Req().IsExistsParameter(name);
    }

    std::string const& header(std::string const& field) const
    {
        return this->Req().GetHeader(field);
    }

    bool is_exists_header(std::string const& field) const
    {
        return this->Req().IsExistsHeader(field);
    }

    std::string const& url_parameter(std::string const& name) const
    {
        return this->Req().GetUrlParameter(name);
    }

    bool is_exists_url_parameter(std::string const& name) const
    {
        return this->Req().IsExistsUrlParameter(name);
    }

    std::string const& form_data(std::string const& name) const
    {
        return this->Req().GetFormData(name);
    }

    bool is_exists_form_data(std::string const& name) const
    {
        return this->Req().IsExistsFormData(name);
    }

    std::string const& cookie(std::string const& name) const
    {
        return this->Req().GetCookie(name);
    }

    bool is_exists_cookie(std::string const& name) const
    {
        return this->Req().IsExistsCookie(name);
    }
private:
    ConnectionPtr _cnt;
    inja::Environment _env;
};


} //namespace da4qi4


#endif // DAQI_CONTEXT_HPP
