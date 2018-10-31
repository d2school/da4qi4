#ifndef DAQI_RENDERER_HPP
#define DAQI_RENDERER_HPP

#include <string>

#include <inja/inja.hpp>

namespace da4qi4
{

class TemplateEngine
{
public:
    TemplateEngine(std::string const& template_root)
        : _root(template_root), _env(template_root)
    {
        _env.set_element_notation(inja::ElementNotation::Dot);
    }

    inja::Environment const& GetEnv() const
    {
        return _env;
    }

    inja::Environment& GetEnv()
    {
        return _env;
    }

    void Preload();
    void Preload(std::string const& template_root)
    {
        _root = template_root;
        Preload();
    }
    inja::Template const* Get(std::string const& name);

private:
    std::map<std::string, inja::Template> _templates;
    std::string _root;
    inja::Environment _env;
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
