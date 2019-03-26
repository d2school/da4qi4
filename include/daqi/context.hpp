#ifndef DAQI_CONTEXT_HPP
#define DAQI_CONTEXT_HPP

#include <memory>
#include <functional>
#include <list>

#include "daqi/def/def.hpp"
#include "daqi/def/asio_def.hpp"
#include "daqi/def/json_def.hpp"

#include "daqi/request.hpp"
#include "daqi/response.hpp"
#include "daqi/templates.hpp"
#include "daqi/intercepter.hpp"
#include "daqi/rediscli_pool.hpp"

namespace da4qi4
{

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class ContextIMP;
using Context = std::shared_ptr<ContextIMP>;

class Application;

class ContextIMP
    : public std::enable_shared_from_this<ContextIMP>
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
    Application const& App() const;

    Json const& Data(std::string const& name) const
    {
        Json::const_iterator it = _data.find(name);

        if (it == _data.cend())
        {
            return theNullJson;
        }

        return *it;
    }

    Json& Data(std::string const& name)
    {
        auto it = _data.find(name);

        if (it != _data.end())
        {
            return *it;
        }

        _data[name] = Json();
        return _data[name];
    }

    Json const& LoadData(std::string const& name) const
    {
        return Data(name);
    }

    void SaveData(std::string const& name, Json const& data)
    {
        _data[name] = data;
    }

    void RemoveData(std::string const& name)
    {
        auto it = _data.find(name);

        if (it != _data.end())
        {
            _data.erase(it);
        }
    }

    Json& ModelData()
    {
        return Data(model_data_name);
    }

    Json const& ModelData() const
    {
        return Data(model_data_name);
    }

    Json const& LoadModelData() const
    {
        return LoadData(model_data_name);
    }

    void SaveModelData(Json const& data)
    {
        SaveData(model_data_name, data);
    }

    Json& SessionData()
    {
        return Data(session_data_name);
    }

    Json const& SessionData() const
    {
        return Data(session_data_name);
    }

    Json LoadSessionData() const
    {
        return LoadData(session_data_name);
    }

    void SaveSessionData(Json const& data)
    {
        SaveData(session_data_name, data);
    }

    void ClearSessionData();

    std::string GetSessionID() const;
    Cookie GetSessionCookie() const;

    static std::string const& ModelDataName()
    {
        return model_data_name;
    }

    static std::string const& SessionDataName()
    {
        return session_data_name;
    }

    IOC& IOContext();
    size_t IOContextIndex() const;

    log::LoggerPtr Logger()
    {
        return logger();
    }

public:
    std::string const& GetTemplateName() const
    {
        return _template_name;
    }

    void SetTemplateName(std::string const& template_name)
    {
        _template_name = template_name;
    }

    void ClearTemplateName()
    {
        _template_name.clear();
    }

public:
    void InitRequestPathParameters(std::vector<std::string> const& names
                                   , std::vector<std::string> const& values);

public:
    ContextIMP& Render();
    ContextIMP& Render(std::string const& template_name, Json const& data = theNullJson);

    ContextIMP& RenderWithData(http_status status, Json const& data);
    ContextIMP& RenderWithData(std::string const& template_name, Json const& data);
    ContextIMP& RenderWithData(Json const& data);

public:
    ContextIMP& RenderWithoutData(http_status status)
    {
        return RenderWithData(status, theNullJson);
    }
    ContextIMP& RenderWithoutData(std::string const& template_name)
    {
        return RenderWithData(template_name, theNullJson);
    }
    ContextIMP& RenderWithoutData()
    {
        return RenderWithData(theNullJson);
    }

public:
    ContextIMP& RenderNofound(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_NOT_FOUND, data);
    }

    ContextIMP& RenderBadRequest(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_BAD_REQUEST, data);
    }

    ContextIMP& RenderUnauthorized(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_UNAUTHORIZED, data);
    }

    ContextIMP& RenderForbidden(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_FORBIDDEN, data);
    }

    ContextIMP& RenderNotImplemented(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_NOT_IMPLEMENTED, data);
    }

    ContextIMP& RenderServiceUnavailable(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_SERVICE_UNAVAILABLE, data);
    }

    ContextIMP& RenderInternalServerError(Json const& data = theNullJson)
    {
        return RenderWithData(HTTP_STATUS_INTERNAL_SERVER_ERROR, data);
    }

public:
    ContextIMP&  Redirect(std::string const& dst_location)
    {
        this->Res().ReplyRedirect(dst_location);
        return *this;
    }

public:
    bool HasRedis() const
    {
        return _redis != nullptr;
    }

    RedisClientPtr Redis()
    {
        return _redis;
    }

public:
    void Start();
    void Pass();
    void Stop();

public:
    void StartChunkedResponse();
    void NextChunkedResponse(std::string const& body);
    void StopChunkedResponse();

private:
    void do_intercepter_on_req_dir();
    void do_intercepter_on_res_dir();

    void next(Intercepter::Result result);
    void next_intercepter_on_req_dir(Intercepter::Result result);
    void start_intercepter_on_res_dir(Intercepter::Result result);
    void next_intercepter_on_res_dir(Intercepter::Result result);

private:
    void end();

private:
    log::LoggerPtr logger();

private:
    using Self = ContextIMP;
    typedef std::string const& (Self::*PSSFun)(std::string const&) const;
    void RegistStringFunctionWithOneStringParameter(char const* function_name,
                                                    PSSFun func,
                                                    std::string defaultValue = Utilities::theEmptyString
                                                   );

    typedef bool (Self::*PBSFun)(std::string const&) const;
    void RegistBoolFunctionWithOneStringParameter(char const* function_name,
                                                  PBSFun func, bool defaultValue = false);

private:
    void render_on_template(std::string const& templ_name, Template const& templ, Json const& data, http_status status);
    std::string render_on_template(std::string const& templ_name, Template const& templ, Json const& data
                                   , bool& server_render_error
                                   , std::string& error_what);

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

    void prepair_template_env();
    std::string auto_match_template();

private:
    ConnectionPtr _cnt;

    Json _data;
    TemplatesEnv _env; //every context has it's templats env;

    Intercepter::On _intercepter_on;
    Intercepter::ChainIterator  _intercepter_iter;
    Intercepter::ChainIterator _intercepter_beg, _intercepter_end;

    RedisClientPtr _redis;

    std::string _template_name;
private:
    static std::string session_data_name;
    static std::string model_data_name;
};


} //namespace da4qi4


#endif // DAQI_CONTEXT_HPP
