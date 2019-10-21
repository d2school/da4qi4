#include "daqi/router.hpp"

namespace da4qi4
{

router_equals operator "" _router_equals(char const* str, std::size_t n)
{
    return router_equals(std::string(str, n));
}

router_starts operator "" _router_starts(char const* str, std::size_t n)
{
    return router_starts(std::string(str, n));
}

router_regex operator "" _router_regex(char const* str, std::size_t n)
{
    return router_regex(std::string(str, n));
}

std::size_t to_uniform_items(std::string const& url_matcher
                             , RouterItem const& ri, std::vector<UniformItem>& uitems)
{
    for (auto const& h : ri.handlers)
    {
        UniformItem ui(url_matcher);
        ui.method = HandlerMethodName(h.first);
        ui.template_name = ri.template_name;
        uitems.push_back(ui);
    }

    return ri.handlers.size();
}

std::size_t to_uniform_items(RegexRouterItem const& ri, std::vector<UniformRegexItem>& uitems)
{
    for (auto const& h : ri.handlers)
    {
        UniformRegexItem ui(ri.url_matcher);

        ui.method = HandlerMethodName(h.first);
        ui.template_name = ri.template_name;

        ui.regex_matcher = ri.regex_matcher;

        uitems.push_back(ui);
    }

    return ri.handlers.size();
}

std::vector<UniformItem> EqualsRoutingTable::Uniforms() const
{
    std::vector<UniformItem> items;

    for (auto const& p : _map)
    {
        to_uniform_items(p.first, p.second, items);
    }

    return items;
}

StartsWithRoutingTable::Result StartsWithRoutingTable::Match(std::string const& url
                                                             , HandlerMethod method
                                                             , bool& url_exists)
{
    auto it = _map.begin();

    for (; it != _map.end(); ++it)
    {
        if (Utilities::iStartsWith(url, it->first))
        {
            url_exists = true;
            return Result(&(it->second), method, it->first);
        }
    }

    return Result();
}

std::vector<UniformItem> StartsWithRoutingTable::Uniforms() const
{
    std::vector<UniformItem> items;

    for (auto const& p : _map)
    {
        to_uniform_items(p.first, p.second, items);
    }

    return items;
}

std::string to_parameter_pattern(std::string simple_pattern
                                 , std::vector<std::string>& names)
{
    std::regex pattern("\\{\\{\\w*\\}\\}");

    std::string::const_iterator itb = simple_pattern.begin(), ite = simple_pattern.end();

    using PairItem = std::pair<std::string::const_iterator, std::string::const_iterator>;
    std::vector<PairItem> pos;
    std::smatch result;

    while (std::regex_search(itb, ite, result, pattern))
    {
        int const parameter_name_flag = 2;
        std::string name(result[0].first + parameter_name_flag
                         , result[0].second - parameter_name_flag);
        names.push_back(name);

        PairItem it = std::make_pair(result[0].first, result[0].second);
        pos.push_back(it);

        itb = result[0].second;
    }

    for (size_t i = pos.size(); i > 0; --i)
    {
        auto item = pos[i - 1];
        simple_pattern.replace(item.first, item.second, "(\\w*)");
    }

    return simple_pattern;
}

bool RegexMatchRoutingTable::Insert(std::string const&  url_matcher,
                                    RouterItem const& item, std::string& error)
{
    error.clear();

    Item ri(item);
    ri.url_matcher = url_matcher;

    try
    {
        ri.regex_matcher = to_parameter_pattern(url_matcher, ri.parameters);
        ri.regex_pattern.assign(ri.regex_matcher);
        _lst.push_back(ri);
        return true;
    }
    catch (std::regex_error const& e)
    {
        error = e.what();
        return false;
    }
    catch (std::exception const& e)
    {
        error = e.what();
        return false;
    }
}

RegexMatchRoutingTable::Item* RegexMatchRoutingTable::Exists(std::string const& url_matcher)
{
    for (auto& item : _lst)
    {
        if (url_matcher == item.url_matcher)
        {
            return &(item);
        }
    }

    return nullptr;
}

RegexMatchRoutingTable::Result RegexMatchRoutingTable::Match(std::string const& url
                                                             , HandlerMethod method
                                                             , bool& url_exists)
{
    Result r;

    try
    {
        for (auto& item : _lst)
        {
            std::smatch result;

            if (std::regex_match(url, result, item.regex_pattern))
            {
                url_exists = true;
                Result rr(&item, method);

                if (!rr.handler)
                {
                    return Result();
                }

                int const skip_first_one = 1;

                for (size_t i = skip_first_one ; i < result.size(); ++i)
                {
                    std::string value = result[i].str();
                    rr.values.push_back(value);
                }

                rr.parameters = item.parameters;
                return rr;
            }
        }
    }
    catch (std::regex_error const& e)
    {
        r.error = e.what();
        return r;
    }
    catch (std::exception const& e)
    {
        r.error = e.what();
    }

    return r;
}

std::vector<UniformRegexItem> RegexMatchRoutingTable::Uniforms() const
{
    std::vector<UniformRegexItem> items;

    for (auto const& l : _lst)
    {
        to_uniform_items(l, items);
    }

    return items;
}

} //namespace da4qi4

da4qi4::router_equals operator "" _da4qi4_router_equals(char const* str, std::size_t n)
{
    return da4qi4::operator "" _router_equals(str, n);
}

da4qi4::router_starts operator "" _da4qi4_router_starts(char const* str, std::size_t n)
{
    return da4qi4::operator "" _router_starts(str, n);
}

da4qi4::router_regex operator "" _da4qi4_router_regex(char const*  str, std::size_t n)
{
    return da4qi4::operator "" _router_regex(str, n);
}

