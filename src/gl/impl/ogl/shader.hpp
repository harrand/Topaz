#ifndef TOPAZ_GL_IMPL_OGL_SHADER_HPP
#define TOPAZ_GL_IMPL_OGL_SHADER_HPP
#if TZ_OGL
#include "gl/api/shader.hpp"

namespace tz::gl
{
    class ShaderBuilderOGL : public IShaderBuilder
    {
    public:
        ShaderBuilderOGL() = default;
        virtual void set_shader_file(ShaderType type, std::filesystem::path shader_file) final;
        virtual void set_shader_source(ShaderType type, std::string source_code) final;
        virtual std::string_view get_shader_source(ShaderType type) const final;
        virtual bool has_shader(ShaderType type) const final;
    };

    class ShaderOGL
    {

    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_SHADER_HPP