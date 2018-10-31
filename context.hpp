#ifndef DAQI_CONTEXT_HPP
#define DAQI_CONTEXT_HPP

#include <memory>
#include <functional>
#include <list>

#include "request.hpp"
#include "response.hpp"

namespace da4qi4
{

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

    Request const& Req();
    Response& Res();

    std::string const& Req(std::string const& name)
    {
        return this->Req()[name];
    }

    Application& App();

    void Bye();

private:
    ConnectionPtr _cnt;
};


} //namespace da4qi4


#endif // DAQI_CONTEXT_HPP
