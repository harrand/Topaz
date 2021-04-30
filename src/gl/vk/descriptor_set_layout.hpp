#ifndef TOPAZ_GL_VK_DESCRIPTOR_SET_LAYOUT_HPP
#define TOPAZ_GL_VK_DESCRIPTOR_SET_LAYOUT_HPP
#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include <cstdint>

namespace tz::gl::vk
{
    enum class DescriptorType
    {
        UniformBuffer
    };

    struct DescriptorSetLayoutBinding
    {
        std::uint32_t binding;
        DescriptorType type;
    };

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(const LogicalDevice& device, DescriptorSetLayoutBinding binding);
        DescriptorSetLayout(const DescriptorSetLayout& copy) = delete;
        DescriptorSetLayout(DescriptorSetLayout&& move);
        ~DescriptorSetLayout();

        DescriptorSetLayout& operator=(const DescriptorSetLayout& rhs) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& rhs);

        VkDescriptorSetLayout native() const;
        DescriptorSetLayoutBinding get_binding() const;
    private:
        VkDescriptorSetLayout layout;
        DescriptorSetLayoutBinding binding;
        const LogicalDevice* device;
    };

    using DescriptorSetLayouts = std::vector<DescriptorSetLayout>;
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_DESCRIPTOR_SET_LAYOUT_HPP