#ifndef DAQI_TEMPLATES_HPP
#define DAQI_TEMPLATES_HPP

#include <string>
#include <unordered_map>

#include <inja/inja.hpp>

namespace da4qi4
{

class TemplateLibrary
{
public:
    TemplateLibrary(std::string const& template_root)
        : _root(template_root)
    {
    }

    size_t Preload();
    size_t Preload(std::string const& template_root)
    {
        _root = template_root;
        return Preload();
    }
    inja::Template const* Get(std::string const& name);

private:
    size_t load_templates(std::string const& template_ext, std::string const& key_ext);
    bool try_load_template(std::string const& key, std::string const& template_filename);

    std::unordered_map<std::string, inja::Template> _templates;
    std::string _root;
};

}

#endif // DAQI_TEMPLATE_LIBRARY_HPP
