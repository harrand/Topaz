#ifndef TOPAZ_UTIL_TZGLP_HPP
#define TOPAZ_UTIL_TZGLP_HPP
#include "gl/shader_preprocessor.hpp"
#include "gl/object.hpp"
#include <string_view>
#include <vector>
#include <string>

template<typename... Args>
void print_error(const char* fmt, Args&&... args)
{
    std::string output;
    output += "\033[1;31m";
    output += fmt;
    output += "\033[0m";
    std::fprintf(stderr, output.c_str(), std::forward<Args>(args)...);
}

struct PreprocessorArgs
{
    std::vector<std::string> module_names;
    std::string shader_source;
    std::string include_path;
};

class TZGLP
{
public:
    TZGLP(const PreprocessorArgs& args);
    std::string get_result() const;
    bool error() const;
private:
    void handle_args(const PreprocessorArgs& args);
    void add_module(std::string_view module_name);
    PreprocessorArgs args;
    tz::gl::Object dummy;
    tz::gl::ShaderPreprocessor preprocessor;
    bool erroneous = false;
};

#endif // TOPAZ_UTIL_TZGLP_HPP