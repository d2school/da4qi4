#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <map>
#include <list>
#include <regex>

#include "utilities/string_utilities.hpp"
#include "handler.hpp"

namespace da4qi4
{

struct RouterItem
{
    std::map<HandlerMethod, Handler> handlers;
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

router_equals operator "" _router_equals(char const* str, size_t n);
router_starts operator "" _router_starts(char const* str, size_t n);
router_regex operator "" _router_regex(char const* str, size_t n);

template<typename IMP>
class RoutingTable
{
private:
    IMP* imp()
    {
        return static_cast<IMP*>(this);
    }
    
public:
    bool Add(std::string const& url_matcher, HandlerMethod method,  Handler handler)
    {
        if (url_matcher.empty())
        {
            return false;
        }
        
        RouterItem* item = imp()->Exists(url_matcher);
        
        if (!item)
        {
            RouterItem ri;
            ri.handlers[method] = handler;
            return imp()->Insert(url_matcher, ri);
        }
        else
        {
            item->handlers[method] = handler;
            return true;
        }
    }
    
    bool Add(std::string const& url_matcher, HandlerMethods methods, Handler handler)
    {
        for (size_t i = 0; i < methods.mark.size(); ++i)
        {
            if (methods.mark[i])
            {
                HandlerMethod method = static_cast<HandlerMethod>(i);
                
                if (!this->Add(url_matcher, method, handler))
                {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    Handler Search(std::string const& url, HandlerMethod method)
    {
        if (auto item = imp()->Match(url))
        {
            auto it = item->handlers.find(method);
            
            if (it != item->handlers.end())
            {
                return  it->second;
            }
        }
        
        return theEmptyHandler;
    }
};

class EqualsRoutingTable : public RoutingTable<EqualsRoutingTable>
{
    using Map = std::map<std::string, RouterItem, Utilities::IgnoreCaseCompare>;
public:
    using Item = RouterItem;
    
    bool Insert(std::string const&  url_matcher, RouterItem const& item)
    {
        _map.insert(std::make_pair(url_matcher, item));
        return true;
    }
    
    Item* Exists(std::string const& url_matcher)
    {
        return Match(url_matcher);
    }
    
    Item* Match(std::string const& url)
    {
        auto it = _map.find(url);
        return (it == _map.end()) ? nullptr : &(it->second);
    }
    
private:
    Map _map;
};

class StartsWithRoutingTable : public RoutingTable<StartsWithRoutingTable>
{
    using Map = std::map<std::string, RouterItem, Utilities::IgnoreCaseCompareDESC>;
public:
    using Item = RouterItem;
    
    bool Insert(std::string const&  url_matcher, RouterItem const& item)
    {
        _map.insert(std::make_pair(url_matcher, item));
        return true;
    }
    
    Item* Exists(std::string const& url_matcher)
    {
        auto it = _map.find(url_matcher);
        return (it == _map.end() ? nullptr : & (it->second));
    }
    
    Item* Match(std::string const& url);
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

class RegexMatchRoutingTable : public RoutingTable<RegexMatchRoutingTable>
{
private:
    using List = std::list <RegexRouterItem>;
    
public:
    using Item = RegexRouterItem;
    
    bool Insert(std::string const&   url_matcher, RouterItem const& item);
    Item* Exists(std::string const& url_matcher);
    Item* Match(std::string const& url);
    
private:
    List _lst;
};

} //namespace da4qi4

#endif // ROUTER_HPP
