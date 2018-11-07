#ifndef DAQI_CONTEXT_HPP
#define DAQI_CONTEXT_HPP

#include <memory>
#include <functional>
#include <list>

#include "def/def.hpp"

#include "request.hpp"
#include "response.hpp"
#include "templates.hpp"
#include "intercepter.hpp"

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
    void InitRequestPathParameters(std::vector<std::string> const& names
                                   , std::vector<std::string> const& values);
public:
    void Render(http_status status, Json const& data);
    void Render(std::string const& template_name, Json const& data);
    void Render(Json const& data);

public:
    void RenderWithoutData(http_status status)
    {
        Render(status, theEmptyJson);
    }
    void RenderWithoutData(std::string const& template_name)
    {
        Render(template_name, theEmptyJson);
    }
    void RenderWithoutData()
    {
        Render(theEmptyJson);
    }

public:
    void RenderNofound(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_NOT_FOUND, data);
    }
    void RenderUnauthorized(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_UNAUTHORIZED, data);
    }

    void RenderForbidden(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_FORBIDDEN, data);
    }

    void RenderNotImplemented(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_NOT_IMPLEMENTED, data);
    }

    void RenderServiceUnavailable(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_SERVICE_UNAVAILABLE, data);
    }

    void RenderInternalServerError(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_INTERNAL_SERVER_ERROR, data);
    }

public:
    void Bye();

public:
    void StartChunkedResponse();
    void ContinueChunkedResponse(std::string const& body);

public:
    void SetIntercepterIterator(Intercepter::ChainIterator iter)
    {
        _intercepter_iter = iter;
    }

    Intercepter::ChainIterator GetIntercepterIterator() const
    {
        return _intercepter_iter;
    }

private:
    void end_chunked_response();
private:
    void render_on_template(Template const& templ, Json const& data, http_status status);
    std::string render_on_template(Template const& templ, Json const& data
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

    std::string const& path_parameter(std::string const& name) const
    {
        return this->Req().GetPathParameter(name);
    }

    bool is_exists_path_parameter(std::string const& name) const
    {
        return this->Req().IsExistsPathParameter(name);
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
    Intercepter::ChainIterator _intercepter_iter;
};


} //namespace da4qi4


#endif // DAQI_CONTEXT_HPP
