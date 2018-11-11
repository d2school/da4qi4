#ifndef DAQI_TEMPLATES_HPP
#define DAQI_TEMPLATES_HPP

#include <string>
#include <unordered_map>

#include "def/inja_def.hpp"

namespace da4qi4
{

class Templates
{
public:
    Templates(std::string const& template_root, std::string const& app_url_root)
        : _root(template_root), _app_prefix(app_url_root)
    {
        if (!_app_prefix.empty() && _app_prefix[0] == '/')
        {
            _app_prefix = _app_prefix.substr(1);
        }
    }

    size_t Preload();

    Template const* Get(std::string const& name);

private:
    size_t load_templates(std::string const& template_ext, std::string const& key_ext);
    bool try_load_template(std::string const& key, std::string const& template_filename);

    std::unordered_map<std::string, Template> _templates;
    std::string _root, _app_prefix;
};

}

#endif // DAQI_TEMPLATE_LIBRARY_HPP
