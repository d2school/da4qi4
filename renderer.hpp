#ifndef DAQI_RENDERER_HPP
#define DAQI_RENDERER_HPP

#include <string>

#include <inja/inja.hpp>

namespace da4qi4
{

class TemplateLibrary
{
public:
    TemplateLibrary(std::string const& template_root)
        : _root(template_root)
    {}

    size_t Preload();
    inja::Template const* const const& Get(std::string const& name);

private:
    std::map<std::string, inja::Template> _templates;
    std::string _root;
};

class ContextIMP;

class Render
{
public:
    Render(ContextIMP* ctx)
        : _ctx(ctx)
    {
    }

private:
    ContextIMP* _ctx;
};

}

#endif // DAQI_RENDERER_HPP
