#ifndef TOPAZ_GL_IMPL_VK_SHADER_HPP
#define TOPAZ_GL_IMPL_VK_SHADER_HPP
#if TZ_VULKAN
#include "gl/api/shader.hpp"

#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/pipeline/shader_module.hpp"

namespace tz::gl
{
    class ShaderBuilderVulkan
    {
    public:
        ShaderBuilderVulkan() = default;
        virtual void set_shader_file(ShaderType type, std::filesystem::path shader_file) final;
        virtual void set_shader_source(ShaderType type, std::string source_code) final;
        virtual void set_shader_meta(ShaderType type, std::string metadata) final;
        virtual std::string_view get_shader_meta(ShaderType type) const final;
        virtual std::string_view get_shader_source(ShaderType type) const final;
        virtual bool has_shader(ShaderType type) const final;
    private:
        std::string vertex_shader_source;
        std::string fragment_shader_source;
        std::string compute_shader_source;

        std::string vertex_shader_metadata;
        std::string fragment_shader_metadata;
        std::string compute_shader_metadata;
    };

    class ShaderVulkan
    {
    public:
        ShaderVulkan(const vk::LogicalDevice& device, ShaderBuilderVulkan builder);
        const vk::ShaderModule& vk_get_vertex_shader() const;
        const vk::ShaderModule& vk_get_fragment_shader() const;
        const vk::ShaderModule& vk_get_compute_shader() const;
        const ShaderMeta& vk_get_meta() const;
    private:
        std::optional<vk::ShaderModule> vertex_shader = std::nullopt;
        std::optional<vk::ShaderModule> fragment_shader = std::nullopt;
        std::optional<vk::ShaderModule> compute_shader = std::nullopt;
        ShaderMeta meta;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_VK_SHADER_HPP