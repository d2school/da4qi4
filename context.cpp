#include "context.hpp"

#include "connection.hpp"
#include "application.hpp"

namespace da4qi4
{

Context ContextIMP::Make(ConnectionPtr cnt)
{
    return std::shared_ptr<ContextIMP>(new ContextIMP(cnt));
}

ContextIMP::ContextIMP(ConnectionPtr cnt)
    : _cnt(cnt)
{
}

ContextIMP::~ContextIMP()
{

}

Request const& ContextIMP::Req()
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

void ContextIMP::Bye()
{
    _cnt->Write();
}

} //namespace da4qi4
