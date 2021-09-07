#if TZ_OGL
#include "core/assert.hpp"
#include "gl/impl/frontend/ogl/shader.hpp"
#include <fstream>
#include <filesystem>

namespace tz::gl
{
    ShaderOGL::ShaderOGL(ShaderBuilderOGL builder):
    program(glCreateProgram()),
    vertex_shader(0),
    fragment_shader(0),
    compute_shader(0),
    meta(),
    is_compute(!builder.get_shader_source(ShaderType::ComputeShader).empty())
    {
        auto setup_shader = [this, &builder](ShaderType type)->std::string
        {
            GLuint* shad;
            GLenum shad_type;
            switch(type)
            {
                case ShaderType::VertexShader:
                    shad = &this->vertex_shader;
                    shad_type = GL_VERTEX_SHADER;
                break;
                case ShaderType::FragmentShader:
                    shad = &this->fragment_shader;
                    shad_type = GL_FRAGMENT_SHADER;
                break;
                case ShaderType::ComputeShader:
                    shad = &this->compute_shader;
                    shad_type = GL_COMPUTE_SHADER;
                break;
                default:
                    tz_error("Unknown ShaderType");
                break;
            }

            *shad = glCreateShader(shad_type);
            glAttachShader(this->program, *shad);
            // Upload source
            {
                std::string_view src = builder.get_shader_source(type);
                auto src_len = static_cast<GLint>(src.length());
                const GLchar* src_data = src.data();
                glShaderSource(*shad, 1, &src_data, &src_len);
            }
            // Compile
            glCompileShader(*shad);
            ShaderOGL::check_shader_error(*shad);
            // Metadata
            return std::string("\n") + static_cast<std::string>(builder.get_shader_meta(type));
        };
        std::string all_metadata;
        if(this->is_compute)
        {
            all_metadata = setup_shader(ShaderType::ComputeShader);
        }
        else
        {
            all_metadata = setup_shader(ShaderType::VertexShader);
            all_metadata += setup_shader(ShaderType::FragmentShader);
        }
        // Link & Validate
        glLinkProgram(this->program);
        ShaderOGL::check_program_error(this->program);
        glValidateProgram(this->program);
        ShaderOGL::check_program_error(this->program);

        // Meta
        this->meta = ShaderMeta::from_metadata_string(all_metadata);
    }

    ShaderOGL::ShaderOGL(ShaderOGL&& move):
    program(0),
    vertex_shader(0),
    fragment_shader(0),
    compute_shader(0),
    is_compute(false)
    {
        *this = std::move(move);
    }

    ShaderOGL::~ShaderOGL()
    {
        if(!is_compute)
        {
            glDetachShader(this->program, this->vertex_shader);
            glDetachShader(this->program, this->fragment_shader);
        }
        else
        {
            glDetachShader(this->program, this->compute_shader);
        }
        glDeleteShader(this->vertex_shader);
        glDeleteShader(this->fragment_shader);
        glDeleteShader(this->compute_shader);
        glDeleteProgram(this->program);
        this->program = 0;
        this->vertex_shader = 0;
        this->fragment_shader = 0;
    }

    ShaderOGL& ShaderOGL::operator=(ShaderOGL&& rhs)
    {
        std::swap(this->program, rhs.program);
        std::swap(this->vertex_shader, rhs.vertex_shader);
        std::swap(this->fragment_shader, rhs.fragment_shader);
        std::swap(this->compute_shader, rhs.compute_shader);
        std::swap(this->meta, rhs.meta);
        std::swap(this->is_compute, rhs.is_compute);
        return *this;
    }

    GLuint ShaderOGL::ogl_get_program_handle() const
    {
        return this->program;
    }

    const ShaderMeta& ShaderOGL::get_meta() const
    {
        return this->meta;
    }

    void ShaderOGL::check_shader_error(GLuint shader)
    {
        GLint cpl_status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &cpl_status);
        tz_assert(cpl_status == GL_TRUE, "Shader Compile Error: %s", ShaderOGL::shad_info_log(shader).c_str());
    }

    void ShaderOGL::check_program_error(GLuint program)
    {
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        tz_assert(status == GL_TRUE, "Shader Program Link Error: %s", ShaderOGL::prog_info_log(program).c_str());
        glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
        tz_assert(status == GL_TRUE, "Shader Program Validate Error: %s", ShaderOGL::prog_info_log(program).c_str());
    }

    std::string ShaderOGL::shad_info_log(GLuint shader)
    {
        GLint info_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
        GLsizei real_log_size;
        std::string info_log;
        info_log.resize(info_log_length);
        glGetShaderInfoLog(shader, info_log_length, &real_log_size, info_log.data());
        info_log.resize(real_log_size);
        return info_log;
    }

    std::string ShaderOGL::prog_info_log(GLuint program)
    {
        GLint info_log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
        GLsizei real_log_size;
        std::string info_log;
        info_log.resize(info_log_length);
        glGetProgramInfoLog(program, info_log_length, &real_log_size, info_log.data());
        info_log.resize(real_log_size);
        return info_log;
    }
}

#endif // TZ_OGL