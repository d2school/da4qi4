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

void ByeOnSuccess(Context ctx)
{
    StopOnSuccess(ctx);;
    ctx->Bye();
}

void ByeOnError(Context ctx)
{
    StopOnError(ctx);;
    ctx->Bye();
}

} //Intercepter
} //namespace da4qi4
