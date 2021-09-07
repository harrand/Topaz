#ifndef TOPAZ_GL_IMPL_VK_SHADER_HPP
#define TOPAZ_GL_IMPL_VK_SHADER_HPP
#if TZ_VULKAN
#include "gl/impl/frontend/common/shader.hpp"

#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/pipeline/shader_module.hpp"

namespace tz::gl
{
    using ShaderBuilderVulkan = ShaderBuilderBase;

    class ShaderVulkan : public IShader
    {
    public:
        ShaderVulkan(const vk::LogicalDevice& device, ShaderBuilderVulkan builder);
        const vk::ShaderModule& vk_get_vertex_shader() const;
        const vk::ShaderModule& vk_get_fragment_shader() const;
        const vk::ShaderModule& vk_get_compute_shader() const;
        virtual const ShaderMeta& get_meta() const final;
    private:
        std::optional<vk::ShaderModule> vertex_shader = std::nullopt;
        std::optional<vk::ShaderModule> fragment_shader = std::nullopt;
        std::optional<vk::ShaderModule> compute_shader = std::nullopt;
        ShaderMeta meta;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_VK_SHADER_HPP