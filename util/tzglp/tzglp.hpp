#ifndef TOPAZ_UTIL_TZGLP_HPP
#define TOPAZ_UTIL_TZGLP_HPP
#include "gl/shader_preprocessor.hpp"
#include <vector>
#include <string>

struct PreprocessorArgs
{
    std::vector<std::string> module_names;
    std::string shader_source;
};

class TZGLP
{
public:
    TZGLP(const PreprocessorArgs& args);
    std::string get_result() const;
private:
    void handle_args(const PreprocessorArgs& args);
    tz::gl::ShaderPreprocessor preprocessor;
};

#endif // TOPAZ_UTIL_TZGLP_HPP