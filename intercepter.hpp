#ifndef INTERCEPTER_HPP
#define INTERCEPTER_HPP

#include "def/def.hpp"
#include "def/debug_def.hpp"
#include "handler.hpp"
#include "context.hpp"

namespace da4qi4
{
namespace Intercepter
{

void Pass(Context ctx);
void StopOnSuccess(Context ctx);
void StopOnError(Context ctx);

} //namespace Intercepter
} //namespace da4qi4

#endif // INTERCEPTER_HPP
