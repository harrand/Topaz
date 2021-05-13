#if TZ_OGL
#include "core/assert.hpp"
#include "gl/impl/ogl/shader.hpp"
#include <fstream>

namespace tz::gl
{
    void ShaderBuilderOGL::set_shader_file(ShaderType type, std::filesystem::path shader_file)
    {
        std::ifstream shader{shader_file.c_str(), std::ios::ate};
        tz_assert(shader.is_open(), "Cannot open shader file %s", shader_file.c_str());
        auto file_size_bytes = static_cast<std::size_t>(shader.tellg());
        shader.seekg(0);
        std::string buffer;
        buffer.resize(file_size_bytes);
        shader.read(buffer.data(), file_size_bytes);
        shader.close();
        this->set_shader_source(type, buffer);
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