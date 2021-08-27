#if TZ_OGL
#include "core/assert.hpp"
#include "gl/impl/frontend/ogl/shader.hpp"
#include <fstream>
#include <filesystem>

namespace tz::gl
{
    void ShaderBuilderOGL::set_shader_file(ShaderType type, std::filesystem::path shader_file)
    {
        auto read_all = [](std::filesystem::path path)->std::string
        {
            std::ifstream fstr(path.c_str(), std::ios::ate | std::ios::binary);
            if(!fstr.is_open())
            {
                return "";
            }
            auto file_size_bytes = static_cast<std::size_t>(fstr.tellg());
            fstr.seekg(0);
            std::string buffer;
            buffer.resize(file_size_bytes);
            fstr.read(buffer.data(), file_size_bytes);
            fstr.close();
            return buffer;
        };
        std::filesystem::path glsl_file = shader_file;
        glsl_file += ".glsl";
        this->set_shader_source(type, read_all(glsl_file));

        // Now find the meta file if there is one
        std::filesystem::path meta_file = shader_file;
        meta_file += ".glsl.meta";
        if(std::filesystem::exists(meta_file))
        {
            this->set_shader_meta(type, read_all(meta_file));
        }
    }

    void ShaderBuilderOGL::set_shader_source(ShaderType type, std::string source_code)
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                this->vertex_shader_source = source_code;
            break;
            case ShaderType::FragmentShader:
                this->fragment_shader_source = source_code;
            break;
            default:
                tz_error("Shader type (write) is not supported on Vulkan");
            break;
        }
    }

    void ShaderBuilderOGL::set_shader_meta(ShaderType type, std::string metadata)
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                this->vertex_shader_metadata = metadata;
            break;
            case ShaderType::FragmentShader:
                this->fragment_shader_metadata = metadata;
            break;
            default:
                tz_error("Shader type (write) is not supported on Vulkan");
            break;
        }
    }

    std::string_view ShaderBuilderOGL::get_shader_source(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return this->vertex_shader_source;
            break;
            case ShaderType::FragmentShader:
                return this->fragment_shader_source;
            break;
            default:
                tz_error("Shader type (write) is not supported on Vulkan");
                return "";
            break;
        }
    }

    std::string_view ShaderBuilderOGL::get_shader_meta(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return this->vertex_shader_metadata;
            break;
            case ShaderType::FragmentShader:
                return this->fragment_shader_metadata;
            break;
            default:
                tz_error("Shader type (write) is not supported on Vulkan");
                return "";
            break;
        }
    }

    bool ShaderBuilderOGL::has_shader([[maybe_unused]] ShaderType type) const
    {
        return false;
    }

    ShaderOGL::ShaderOGL(ShaderBuilderOGL builder):
    program(glCreateProgram()),
    vertex_shader(glCreateShader(GL_VERTEX_SHADER)),
    fragment_shader(glCreateShader(GL_FRAGMENT_SHADER)),
    meta()
    {
        // Attach shaders
        glAttachShader(this->program, this->vertex_shader);
        glAttachShader(this->program, this->fragment_shader);

        // Upload source code
        {
            const GLchar* vtx_src = builder.get_shader_source(ShaderType::VertexShader).data();
            const GLchar* frg_src = builder.get_shader_source(ShaderType::FragmentShader).data();
            glShaderSource(this->vertex_shader, 1, &vtx_src, nullptr);
            glShaderSource(this->fragment_shader, 1,&frg_src , nullptr);
        }
        // Compile
        glCompileShader(this->vertex_shader);
        ShaderOGL::check_shader_error(this->vertex_shader);
        glCompileShader(this->fragment_shader);
        ShaderOGL::check_shader_error(this->fragment_shader);
        // Link
        glLinkProgram(this->program);
        glValidateProgram(this->program);
        ShaderOGL::check_program_error(this->program);

        // Meta
        std::string all_metadata;
        {
            all_metadata += std::string("\n") + static_cast<std::string>(builder.get_shader_meta(ShaderType::VertexShader));
            all_metadata += std::string("\n") + static_cast<std::string>(builder.get_shader_meta(ShaderType::FragmentShader));
        }
        this->meta = ShaderMeta::from_metadata_string(all_metadata);
    }

    ShaderOGL::ShaderOGL(ShaderOGL&& move):
    program(0),
    vertex_shader(0),
    fragment_shader(0)
    {
        *this = std::move(move);
    }

    ShaderOGL::~ShaderOGL()
    {
        glDetachShader(this->program, this->vertex_shader);
        glDetachShader(this->program, this->fragment_shader);
        glDeleteShader(this->vertex_shader);
        glDeleteShader(this->fragment_shader);
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
        return *this;
    }

    GLuint ShaderOGL::ogl_get_program_handle() const
    {
        return this->program;
    }

    const ShaderMeta& ShaderOGL::ogl_get_meta() const
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