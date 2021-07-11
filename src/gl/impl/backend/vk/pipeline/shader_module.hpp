#ifndef TOPAZ_GL_VK_PIPELINE_SHADER_MODULE_HPP
#define TOPAZ_GL_VK_PIPELINE_SHADER_MODULE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"
#include <span>
#include <string>

namespace tz::gl::vk
{
    class ShaderModule
    {
    public:
        ShaderModule(const LogicalDevice& device, const std::span<const char>& spirv);
        ShaderModule(const ShaderModule& copy) = delete;
        ShaderModule(ShaderModule&& move);
        ~ShaderModule();

        ShaderModule& operator=(const ShaderModule& rhs) = delete;
        ShaderModule& operator=(ShaderModule&& rhs);

        VkShaderModule native() const;
    private:
        const LogicalDevice* device;
        VkShaderModule shader_module;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_SHADER_MODULE_HPP