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
    std::string template_name;
};

struct RouterResult
{
    RouterResult() = default;

    RouterResult(RouterItem* item, HandlerMethod method)
    {
        if (item)
        {
            template_name = item->template_name;

            auto it = item->handlers.find(method);

            if (it != item->handlers.end())
            {
                handler = &(it->second);
            }
        }
    }

    Handler* handler = nullptr;
    std::string template_name;
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

struct router_regex
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

template<typename IMP, typename Result, bool try_auto_make_template_name>
class RoutingTable
{
private:
    IMP* imp()
    {
        return static_cast<IMP*>(this);
    }

public:
    bool Add(std::string const& url_matcher, HandlerMethod method,  Handler handler,
             std::string const& template_name, std::string& error)
    {
        assert(!url_matcher.empty());

        typename IMP::Item* item = imp()->Exists(url_matcher);

        if (!item)
        {
            typename IMP::Item ri;

            ri.template_name = template_name;

            if (ri.template_name.empty() && try_auto_make_template_name)
            {
                ri.template_name = url_matcher;
            }

            if (!ri.template_name.empty() && *(--ri.template_name.end()) == '/')
            {
                ri.template_name += "index";
            }

            ri.handlers[method] = handler;
            return imp()->Insert(url_matcher, ri, error);
        }
        else
        {
            item->handlers[method] = handler;
            return true;
        }
    }

    bool Add(std::string const& url_matcher, HandlerMethods methods, Handler handler,
             std::string const& template_name, std::string& error)
    {
        for (size_t i = 0; i < methods.mark.size(); ++i)
        {
            if (methods.mark[i])
            {
                HandlerMethod method = static_cast<HandlerMethod>(i);

                std::string const* p_template_name = (template_name.empty() ?
                                                      &url_matcher : &template_name);

                if (!this->Add(url_matcher, method, handler, *p_template_name, error))
                {
                    return false;
                }
            }
        }

        return true;
    }

    Result Search(std::string const& url, HandlerMethod method, bool& url_exists)
    {
        return imp()->Match(url, method, url_exists);
    }
};

class EqualsRoutingTable : public RoutingTable<EqualsRoutingTable, RouterResult, true>
{
    using Map = std::map<std::string, RouterItem, Utilities::IgnoreCaseCompare>;
public:
    using Item = RouterItem;
    using Result = RouterResult;

    bool Insert(std::string const&  url_matcher, RouterItem const& item, std::string& error)
    {
        error.clear();
        _map.insert(std::make_pair(url_matcher, item));
        return true;
    }

    Item* Exists(std::string const& url_matcher)
    {
        auto it = _map.find(url_matcher);
        return (it == _map.end() ? nullptr : & (it->second));
    }

    Result Match(std::string const& url, HandlerMethod method, bool& url_exists)
    {
        auto item = this->Exists(url);
        url_exists = (item != nullptr);
        return Result(item, method);
    }

private:
    Map _map;
};

struct StartsRouterResult : public RouterResult
{
    StartsRouterResult() = default;
    StartsRouterResult(RouterItem* item, HandlerMethod method, std::string const& key)
        : RouterResult(item, method), key(key)
    {}

    std::string key;
};

class StartsWithRoutingTable
    : public RoutingTable<StartsWithRoutingTable, StartsRouterResult, false>
{
    using Map = std::map<std::string, RouterItem, Utilities::IgnoreCaseCompareDESC>;
public:
    using Item = RouterItem;
    using Result = StartsRouterResult;

    bool Insert(std::string const&  url_matcher, RouterItem const& item, std::string& error)
    {
        error.clear();
        _map.insert(std::make_pair(url_matcher, item));
        return true;
    }

    Item* Exists(std::string const& url_matcher)
    {
        auto it = _map.find(url_matcher);
        return (it == _map.end() ? nullptr : & (it->second));
    }

    Result Match(std::string const& url, HandlerMethod method, bool& url_exists);

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

class RegexMatchRoutingTable :
    public RoutingTable<RegexMatchRoutingTable, RegexRouterResult, false>
{
private:
    using List = std::list <RegexRouterItem>;

public:
    using Item = RegexRouterItem;
    using Result = RegexRouterResult;

    bool Insert(std::string const&    url_matcher, RouterItem const& item, std::string& error);
    Item* Exists(std::string const& url_matcher);
    Result Match(std::string const& url, HandlerMethod method, bool& url_exists);
private:
    List _lst;
};

} //namespace da4qi4

da4qi4::router_equals operator "" _da4qi4_router_equals(char const* str, std::size_t n);
da4qi4::router_starts operator "" _da4qi4_router_starts(char const* str, std::size_t n);
da4qi4::router_regex operator "" _da4qi4_router_regex(char const*  str, std::size_t n);

#endif // DAQI_ROUTER_HPP
