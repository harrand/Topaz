#include "tzglp.hpp"
#include <iostream>

TZGLP::TZGLP(const PreprocessorArgs& args)
{
    this->handle_args(args);
}

std::string TZGLP::get_result() const
{
    return this->preprocessor.result();
}

void TZGLP::handle_args(const PreprocessorArgs& args)
{
    for(const std::string& module_name : args.module_names)
    {
        std::cout << "found module name \"" << module_name << "\"\n";
    }
    this->preprocessor.set_source(args.shader_source);
}
