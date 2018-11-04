#ifndef DAQI_INTERCEPTER_STATIC_FILE_HPP
#define DAQI_INTERCEPTER_STATIC_FILE_HPP

#include <functional>
#include <vector>
#include <string>
#include <map>

#include "intercepter.hpp"

namespace da4qi4
{

namespace Intercepter
{

struct StaticFileIntercepter
{
    StaticFileIntercepter(int max_age_cache_seconds = 0,
                          PathResolve url_resolve_type = PathResolve::is_relative,
                          PathResolve dir_resolve_type = PathResolve::is_relative)
        : _max_age(max_age_cache_seconds)
        , _url_resolve_type(url_resolve_type)
        , _dir_resolve_type(dir_resolve_type)
    {
    }

    StaticFileIntercepter& AddEntry(std::string const& url_root, std::string const& dir_root);
    StaticFileIntercepter& AddDefaultFileName(std::string const& index_filename);
    StaticFileIntercepter& AddDefaultFileNames(std::vector<std::string> const& index_filenames);

    void operator()(Context ctx) const;

    std::vector<std::string> const& GetDefaultFileNames() const
    {
        return _default_filenames;
    }

private:
    int _max_age = 0;
    PathResolve _url_resolve_type, _dir_resolve_type;
    std::map<std::string, std::string, Utilities::CompareDESC> _root_entries;
    std::vector<std::string> _default_filenames;
};

} //namespace Intercepter
} //namespace da4qi4

#endif // DAQI_INTERCEPTER_STATIC_FILE_HPP
