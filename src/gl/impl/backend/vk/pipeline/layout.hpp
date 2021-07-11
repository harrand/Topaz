#ifndef TOPAZ_GL_VK_PIPELINE_LAYOUT_HPP
#define TOPAZ_GL_VK_PIPELINE_LAYOUT_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/descriptor_set_layout.hpp"

namespace tz::gl::vk::pipeline
{
    class Layout
    {
    public:
        Layout(const LogicalDevice& device);
        Layout(const LogicalDevice& device, DescriptorSetLayoutRefs descriptor_layouts);
        Layout(const Layout& copy) = delete;
        Layout(Layout&& move);
        ~Layout();

        Layout& operator=(const Layout& rhs) = delete;
        Layout& operator=(Layout&& rhs);

        VkPipelineLayout native() const;
    private:
        VkPipelineLayout layout;
        DescriptorSetLayoutRefs descriptor_layouts;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_LAYOUT_HPP