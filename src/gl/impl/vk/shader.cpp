#if TZ_VULKAN
#include "core/assert.hpp"
#include "gl/impl/vk/shader.hpp"
#include <fstream>

namespace tz::gl
{
    void ShaderBuilderVulkan::set_shader_file(ShaderType type, std::filesystem::path shader_file)
    {
        std::ifstream shader{shader_file.c_str(), std::ios::ate | std::ios::binary};
        tz_assert(shader.is_open(), "Cannot open shader file %s", shader_file.c_str());
        auto file_size_bytes = static_cast<std::size_t>(shader.tellg());
        shader.seekg(0);
        std::string buffer;
        buffer.resize(file_size_bytes);
        shader.read(buffer.data(), file_size_bytes);
        shader.close();
        this->set_shader_source(type, buffer);
    }

    void ShaderBuilderVulkan::set_shader_source(ShaderType type, std::string source_code)
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

    std::string_view ShaderBuilderVulkan::get_shader_source(ShaderType type) const
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
                tz_error("Shader type (read) is not supported on Vulkan");
                return "";
            break;
        }
    }

    bool ShaderBuilderVulkan::has_shader(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return !this->vertex_shader_source.empty();
            break;
            case ShaderType::FragmentShader:
                return !this->fragment_shader_source.empty();
            break;
            default:
                tz_error("Shader type (exist) is not supported on Vulkan");
                return false;
            break;
        }
    }

    ShaderVulkan::ShaderVulkan(const vk::LogicalDevice& device, ShaderBuilderVulkan builder):
    vertex_shader(device, {builder.get_shader_source(ShaderType::VertexShader).data(), builder.get_shader_source(ShaderType::VertexShader).size()}),
    fragment_shader(device, {builder.get_shader_source(ShaderType::FragmentShader).data(), builder.get_shader_source(ShaderType::FragmentShader).size()})
    {

    }

    const vk::ShaderModule& ShaderVulkan::vk_get_vertex_shader() const
    {
        return this->vertex_shader;
    }
    
    const vk::ShaderModule& ShaderVulkan::vk_get_fragment_shader() const
    {
        return this->fragment_shader;
    }
}

#endif // TZ_VULKAN