#ifndef DAQI_ROUTER_HPP
#define DAQI_ROUTER_HPP

#include <map>
#include <list>
#include <regex>

#include "daqi/utilities/string_utilities.hpp"
#include "daqi/handler.hpp"

namespace da4qi4
{

struct RouterItem
{
    std::map<HandlerMethod, Handler> handlers;
};

struct RouterResult
{
    RouterResult() = default;

    RouterResult(RouterItem* item, HandlerMethod method)
    {
        if (item)
        {
            auto it = item->handlers.find(method);

            if (it != item->handlers.end())
            {
                handler = &(it->second);
            }
        }
    }

    Handler* handler = nullptr;
    std::string error;
};

struct router_equals
{
    explicit router_equals(std::string const& s)
        : s(s)
    {}

    operator std::string& ()
    {
        return s;
    }

    std::string s;
};

struct router_starts
{
    explicit router_starts(std::string const& s)
        : s(s)
    {}

    operator std::string& ()
    {
        return s;
    }

    std::string s;
};

struct  router_regex
{
    explicit router_regex(std::string const& s)
        : s(s)
    {}

    operator std::string&  ()
    {
        return s;
    }

    std::string s;
};

router_equals operator "" _router_equals(char const* str, std::size_t n);
router_starts operator "" _router_starts(char const* str, std::size_t n);
router_regex operator "" _router_regex(char const*  str, std::size_t n);

template<typename IMP, typename Result>
class RoutingTable
{
private:
    IMP* imp()
    {
        return static_cast<IMP*>(this);
    }

public:
    bool Add(std::string const& url_matcher, HandlerMethod method,  Handler handler, std::string& error)
    {
        assert(!url_matcher.empty());

        typename IMP::Item* item = imp()->Exists(url_matcher);

        if (!item)
        {
            typename IMP::Item ri;
            ri.handlers[method] = handler;
            return imp()->Insert(url_matcher, ri, error);
        }
        else
        {
            item->handlers[method] = handler;
            return true;
        }
    }

    bool Add(std::string const& url_matcher, HandlerMethods methods, Handler handler, std::string& error)
    {
        for (size_t i = 0; i < methods.mark.size(); ++i)
        {
            if (methods.mark[i])
            {
                HandlerMethod method = static_cast<HandlerMethod>(i);

                if (!this->Add(url_matcher, method, handler, error))
                {
                    return false;
                }
            }
        }

        return true;
    }

    Result Search(std::string const& url, HandlerMethod method)
    {
        return imp()->Match(url, method);
    }
};

class EqualsRoutingTable : public RoutingTable<EqualsRoutingTable, RouterResult>
{
    using Map = std::map<std::string, RouterItem, Utilities::IgnoreCaseCompare>;
public:
    using Item = RouterItem;
    using Result = RouterResult;

    bool Insert(std::string const&  url_matcher, RouterItem const& item, std::string&)
    {
        _map.insert(std::make_pair(url_matcher, item));
        return true;
    }

    Item* Exists(std::string const& url_matcher)
    {
        auto it = _map.find(url_matcher);
        return (it == _map.end() ? nullptr : & (it->second));
    }

    Result Match(std::string const& url, HandlerMethod method)
    {
        auto item = this->Exists(url);
        return Result(item, method);
    }

private:
    Map _map;
};

class StartsWithRoutingTable : public RoutingTable<StartsWithRoutingTable, RouterResult>
{
    using Map = std::map<std::string, RouterItem, Utilities::IgnoreCaseCompareDESC>;
public:
    using Item = RouterItem;
    using Result = RouterResult;

    bool Insert(std::string const&  url_matcher, RouterItem const& item, std::string&)
    {
        _map.insert(std::make_pair(url_matcher, item));
        return true;
    }

    Item* Exists(std::string const& url_matcher)
    {
        auto it = _map.find(url_matcher);
        return (it == _map.end() ? nullptr : & (it->second));
    }

    Result Match(std::string const& url, HandlerMethod method);

    std::string const& GetError() const
    {
        return Utilities::theEmptyString;
    }

    void ClearError()
    {}
private:
    Map _map;
};

struct RegexRouterItem : public RouterItem
{
    std::string url_matcher;
    std::string regex_matcher;
    std::regex regex_pattern;

    RegexRouterItem() = default;

    explicit RegexRouterItem(RouterItem const& base)
        : RouterItem(base)
    {
    }

    std::vector<std::string> parameters;
};

struct RegexRouterResult : public RouterResult
{
    RegexRouterResult() = default;
    RegexRouterResult(RegexRouterItem* item, HandlerMethod method)
        : RouterResult(static_cast<RouterItem*>(item), method)
    {}

    std::vector<std::string> parameters;
    std::vector<std::string> values;
};

class RegexMatchRoutingTable : public RoutingTable<RegexMatchRoutingTable, RegexRouterResult>
{
private:
    using List = std::list <RegexRouterItem>;

public:
    using Item = RegexRouterItem;
    using Result = RegexRouterResult;

    bool Insert(std::string const&    url_matcher, RouterItem const& item, std::string& error);
    Item* Exists(std::string const& url_matcher);
    Result Match(std::string const& url, HandlerMethod method);
private:
    List _lst;
};

} //namespace da4qi4

#endif // DAQI_ROUTER_HPP
