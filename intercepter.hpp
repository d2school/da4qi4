#ifndef INTERCEPTER_HPP
#define INTERCEPTER_HPP

#include <functional>

#include "def/def.hpp"
#include "def/json_def.hpp"
#include "def/debug_def.hpp"

namespace da4qi4
{

class ContextIMP;
using Context = std::shared_ptr<ContextIMP>;

namespace Intercepter
{

enum class Result { Pass, Stop };
enum class On {Request, Handle, Response};

using Handler = std::function<void (Context ctx, On on)>;
using Chain = std::list<Handler>;

using ChainIterator = Chain::iterator;
using ChainReverseIterator = std::reverse_iterator<Chain::iterator>;

} //namespace Intercepter
} //namespace da4qi4

#endif // INTERCEPTER_HPP
