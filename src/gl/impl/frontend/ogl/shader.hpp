#ifndef TOPAZ_GL_IMPL_OGL_SHADER_HPP
#define TOPAZ_GL_IMPL_OGL_SHADER_HPP
#if TZ_OGL
#include "gl/api/shader.hpp"
#include "gl/impl/frontend/common/shader.hpp"
#include "glad/glad.h"

namespace tz::gl
{
    class ShaderBuilderOGL : public IShaderBuilder
    {
    public:
        ShaderBuilderOGL() = default;
        virtual void set_shader_file(ShaderType type, std::filesystem::path shader_file) final;
        virtual void set_shader_source(ShaderType type, std::string source_code) final;
        virtual void set_shader_meta(ShaderType type, std::string metadata) final;
        virtual std::string_view get_shader_source(ShaderType type) const final;
        virtual std::string_view get_shader_meta(ShaderType type) const final;
        virtual bool has_shader(ShaderType type) const final;
    private:
        std::string vertex_shader_source;
        std::string fragment_shader_source;

        std::string vertex_shader_metadata;
        std::string fragment_shader_metadata;
    };

    class ShaderOGL : public IShader
    {
    public:
        ShaderOGL(ShaderBuilderOGL builder);
        ShaderOGL(const ShaderOGL& copy) = delete;
        ShaderOGL(ShaderOGL&& move);
        ~ShaderOGL();

        ShaderOGL& operator=(const ShaderOGL& rhs) = delete;
        ShaderOGL& operator=(ShaderOGL&& rhs);

        GLuint ogl_get_program_handle() const;
        virtual const ShaderMeta& get_meta() const final;
    private:
        static void check_shader_error(GLuint shader);
        static void check_program_error(GLuint program);
        static std::string shad_info_log(GLuint shader);
        static std::string prog_info_log(GLuint program);

        GLuint program;
        GLuint vertex_shader;
        GLuint fragment_shader;
        ShaderMeta meta;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_SHADER_HPP