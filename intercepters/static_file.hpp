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

struct StaticFile
{
    StaticFile()
        : _cache_max_age(300)
        , _url_resolve_type(PathResolve::is_relative)
        , _dir_resolve_type(PathResolve::is_relative)
    {
    }

    StaticFile(int cache_max_age
               , PathResolve url_resolve_type = PathResolve::is_relative
               , PathResolve dir_resolve_type = PathResolve::is_relative)
        : _cache_max_age(cache_max_age)
        , _url_resolve_type(url_resolve_type)
        , _dir_resolve_type(dir_resolve_type)
    {
    }

    int GetCacheMaxAge() const
    {
        return _cache_max_age;
    }

    StaticFile& SetCacheMaxAge(int seconds)
    {
        _cache_max_age = seconds;
        return *this;
    }

    PathResolve GetUrlResolveType() const
    {
        return _url_resolve_type;
    }

    StaticFile& SetUrlResolveType(PathResolve type)
    {
        _url_resolve_type = type;
        return *this;
    }

    PathResolve GetDirResolveType() const
    {
        return _dir_resolve_type;
    }

    StaticFile& SetDirResolveType(PathResolve type)
    {
        _dir_resolve_type = type;
        return *this;
    }

    StaticFile& AddEntry(std::string const& url_root, std::string const& dir_root);
    StaticFile& AddDefaultFileName(std::string const& index_filename);
    StaticFile& AddDefaultFileNames(std::vector<std::string> const& index_filenames);

    std::vector<std::string> const& GetDefaultFileNames() const
    {
        return _default_filenames;
    }

    void operator()(Context ctx, Next next) const;

private:
    int _cache_max_age;
    PathResolve _url_resolve_type, _dir_resolve_type;
    std::map<std::string, std::string, Utilities::CompareDESC> _root_entries;
    std::vector<std::string> _default_filenames;
};

} //namespace Intercepter
} //namespace da4qi4

#endif // DAQI_INTERCEPTER_STATIC_FILE_HPP
