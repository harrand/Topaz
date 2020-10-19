#include "tzglp.hpp"
#include "gl/modules/ssbo.hpp"
#include "gl/modules/ubo.hpp"
#include "gl/modules/bindless_sampler.hpp"
#include <iostream>

TZGLP::TZGLP(const PreprocessorArgs& args)
{
    this->handle_args(args);
    this->preprocessor.preprocess();
}

std::string TZGLP::get_result() const
{
    return this->preprocessor.result();
}

bool TZGLP::error() const
{
    return this->erroneous;
}

void TZGLP::handle_args(const PreprocessorArgs& args)
{
    for(const std::string& module_name : args.module_names)
    {
        this->add_module(module_name);
    }
    this->preprocessor.set_source(args.shader_source);
}

void TZGLP::add_module(std::string_view module_name)
{
    if(module_name == "include")
    {
        print_error("error: module name \"%s\" not yet implemented\n", module_name.data());
        this->erroneous = true;
    }
    else if(module_name == "ssbo")
    {
        this->preprocessor.emplace_module<tz::gl::p::SSBOModule>(&this->dummy);
    }
    else if(module_name == "ubo")
    {
        this->preprocessor.emplace_module<tz::gl::p::UBOModule>(&this->dummy);
    }
    else if(module_name == "bindless_sampler")
    {
        this->preprocessor.emplace_module<tz::gl::p::BindlessSamplerModule>();
    }
    else
    {
        print_error("error: module name \"%s\" unrecognised\n", module_name.data());
        this->erroneous = true;
    }
}
