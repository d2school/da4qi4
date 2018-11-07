#ifndef INTERCEPTER_HPP
#define INTERCEPTER_HPP

#include <functional>

#include "def/def.hpp"
#include "def/debug_def.hpp"

namespace da4qi4
{

class ContextIMP;
using Context = std::shared_ptr<ContextIMP>;

namespace Intercepter
{

enum class Result {Skip, ByeOnSuccess, ByeOnError };

using Next = std::function<void (Result result)>;
using Handler = std::function<void (Context ctx, Next next)>;
using Chain = std::list<Handler>;
using ChainIterator = Chain::iterator;
using ChainConstIterator = Chain::const_iterator;

} //namespace Intercepter
} //namespace da4qi4

#endif // INTERCEPTER_HPP
