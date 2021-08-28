#if TZ_VULKAN
#include "core/assert.hpp"
#include "gl/impl/frontend/vk/shader.hpp"
#include <fstream>

namespace tz::gl
{
    void ShaderBuilderVulkan::set_shader_file(ShaderType type, std::filesystem::path shader_file)
    {
        auto read_all = [](std::filesystem::path path)->std::string
        {
            std::ifstream fstr{path.c_str(), std::ios::ate | std::ios::binary};
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
        std::filesystem::path spv_file = shader_file;
        spv_file += ".spv";
        this->set_shader_source(type, read_all(spv_file));

        // Now find the meta file if there is one
        std::filesystem::path meta_file = shader_file;
        meta_file += ".glsl.meta";
        if(std::filesystem::exists(meta_file))
        {
            this->set_shader_meta(type, read_all(meta_file));
        }
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
            case ShaderType::ComputeShader:
                this->compute_shader_source = source_code;
            break;
            default:
                tz_error("Shader type (write) is not supported on Vulkan");
            break;
        }
    }

    void ShaderBuilderVulkan::set_shader_meta(ShaderType type, std::string metadata)
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                this->vertex_shader_metadata = metadata;
            break;
            case ShaderType::FragmentShader:
                this->fragment_shader_metadata = metadata;
            break;
            case ShaderType::ComputeShader:
                this->compute_shader_metadata = metadata;
            break;
            default:
                tz_error("Shader type (read) is not supported on Vulkan");
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
            case ShaderType::ComputeShader:
                return this->compute_shader_source;
            break;
            default:
                tz_error("Shader type (read) is not supported on Vulkan");
                return "";
            break;
        }
    }

    std::string_view ShaderBuilderVulkan::get_shader_meta(ShaderType type) const
    {
        switch(type)
        {
            case ShaderType::VertexShader:
                return this->vertex_shader_metadata;
            break;
            case ShaderType::FragmentShader:
                return this->fragment_shader_metadata;
            break;
            case ShaderType::ComputeShader:
                return this->compute_shader_metadata;
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
            case ShaderType::ComputeShader:
                return !this->compute_shader_source.empty();
            break;
            default:
                tz_error("Shader type (exist) is not supported on Vulkan");
                return false;
            break;
        }
    }

    ShaderVulkan::ShaderVulkan(const vk::LogicalDevice& device, ShaderBuilderVulkan builder)
    {
        std::string all_metadata;
        std::string_view compute_source = builder.get_shader_source(ShaderType::ComputeShader);
        if(!compute_source.empty())
        {
            // We have a compute shader!
            this->compute_shader = {device, {compute_source.data(), compute_source.size()}};
            all_metadata += builder.get_shader_meta(ShaderType::ComputeShader);
        }
        else
        {
            // No compute shader. We *must* have a valid vertex and fragment shader
            std::string_view vertex_source = builder.get_shader_source(ShaderType::VertexShader);
            std::string_view fragment_source = builder.get_shader_source(ShaderType::FragmentShader);
            tz_assert(!vertex_source.empty(), "Vertex shader source must be present if there is no compute shader");
            tz_assert(!fragment_source.empty(), "Fragment shader source must be present if there is no compute shader");
            this->vertex_shader = {device, {vertex_source.data(), vertex_source.size()}};
            this->fragment_shader = {device, {fragment_source.data(), fragment_source.size()}};
            all_metadata += std::string("\n") + static_cast<std::string>(builder.get_shader_meta(ShaderType::VertexShader));
            all_metadata += std::string("\n") + static_cast<std::string>(builder.get_shader_meta(ShaderType::FragmentShader));
        }
        this->meta = ShaderMeta::from_metadata_string(all_metadata);
    }

    const vk::ShaderModule& ShaderVulkan::vk_get_vertex_shader() const
    {
        return this->vertex_shader.value();
    }
    
    const vk::ShaderModule& ShaderVulkan::vk_get_fragment_shader() const
    {
        return this->fragment_shader.value();
    }

    const vk::ShaderModule& ShaderVulkan::vk_get_compute_shader() const
    {
        return this->compute_shader.value();
    }

    const ShaderMeta& ShaderVulkan::get_meta() const
    {
        return this->meta;
    }

}

#endif // TZ_VULKAN