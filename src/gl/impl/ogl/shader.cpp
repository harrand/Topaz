#if TZ_OGL
#include "gl/impl/ogl/shader.hpp"

namespace tz::gl
{
    void ShaderBuilderOGL::set_shader_file(ShaderType type, std::filesystem::path shader_file)
    {

    }

    void ShaderBuilderOGL::set_shader_source(ShaderType type, std::string source_code)
    {

    }

    std::string_view ShaderBuilderOGL::get_shader_source(ShaderType type) const
    {
        return "";
    }

    bool ShaderBuilderOGL::has_shader(ShaderType type) const
    {
        return false;
    }
}

#endif // TZ_OGL