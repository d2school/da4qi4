#include "intercepter.hpp"

namespace da4qi4
{
namespace Intercepter
{

void Pass(Context ctx)
{
    ctx->SetInterceptResult(InterceptResult::pass);
}

void StopOnSuccess(Context ctx)
{
    ctx->SetInterceptResult(InterceptResult::stop_on_success);
}

void StopOnError(Context ctx)
{
    ctx->SetInterceptResult(InterceptResult::stop_on_error);
}

} //Intercepter
} //namespace da4qi4
