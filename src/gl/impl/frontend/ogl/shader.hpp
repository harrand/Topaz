#ifndef TOPAZ_GL_IMPL_OGL_SHADER_HPP
#define TOPAZ_GL_IMPL_OGL_SHADER_HPP
#if TZ_OGL
#include "gl/api/shader.hpp"
#include "gl/impl/frontend/common/shader.hpp"
#include "glad/glad.h"

namespace tz::gl
{
    using ShaderBuilderOGL = ShaderBuilderBase;

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
        GLuint compute_shader;
        ShaderMeta meta;

        bool is_compute;
    };
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_OGL_SHADER_HPP