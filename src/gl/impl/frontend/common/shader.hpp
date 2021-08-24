#ifndef TOPAZ_GL_IMPL_COMMON_SHADER_HPP
#define TOPAZ_GL_IMPL_COMMON_SHADER_HPP
#include <string>
#include <map>

namespace tz::gl
{
    enum class ShaderType
    {
        VertexShader,
        FragmentShader,
        ComputeShader
    };

    struct ShaderMeta
    {
        static ShaderMeta from_metadata_string(const std::string& metadata);
        std::map<unsigned int, std::string> resource_types = {};
    };
}

#endif // TOPAZ_GL_IMPL_COMMON_SHADER_HPP