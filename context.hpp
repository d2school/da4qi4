#ifndef DAQI_CONTEXT_HPP
#define DAQI_CONTEXT_HPP

#include <memory>
#include <functional>
#include <list>

#include "def/def.hpp"
#include "def/asio_def.hpp"
#include "def/json_def.hpp"

#include "request.hpp"
#include "response.hpp"
#include "templates.hpp"
#include "intercepter.hpp"
#include "redis_client.hpp"

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

    Json LoadData(std::string const& name) const
    {
        auto it = _data.find(name);

        if (it == _data.end())
        {
            return theEmptyJson;
        }

        return *it;
    }

    void SaveData(std::string const& name, Json const& data)
    {
        assert(!name.empty());
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

    boost::asio::io_context& IOContext();
    size_t IOContextIndex() const;

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

    void RenderBadRequest(Json const& data = theEmptyJson)
    {
        Render(HTTP_STATUS_BAD_REQUEST, data);
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
    bool HasRedis() const
    {
        return _redis != nullptr;
    }

    PersistentSyncRedisClient* SyncRedis()
    {
        return (_redis) ? &(_redis->SyncClient()) : nullptr;
    }

    PersistentAsyncRedisClient* AsyncRedis()
    {
        return (_redis) ? &(_redis->AsyncClient()) : nullptr;
    }

public:
    void Start();
    void Pass();
    void Stop();

public:
    void StartChunkedResponse();
    void ContinueChunkedResponse(std::string const& body);
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

    Json _data;

    inja::Environment _env;

    Intercepter::On _intercepter_on;
    Intercepter::ChainIterator  _intercepter_iter;
    Intercepter::ChainIterator _intercepter_beg, _intercepter_end;

    RedisClientPtr _redis;
};


} //namespace da4qi4


#endif // DAQI_CONTEXT_HPP
