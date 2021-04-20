#ifndef TOPAZ_GL_VK_PIPELINE_LAYOUT_HPP
#define TOPAZ_GL_VK_PIPELINE_LAYOUT_HPP
#if TZ_VULKAN
#include "gl/vk/impl/setup/logical_device.hpp"

namespace tz::gl::vk::pipeline
{
    class Layout
    {
    public:
        Layout(const LogicalDevice& device);
        Layout(const Layout& copy) = delete;
        Layout(Layout&& move);
        ~Layout();

        Layout& operator=(const Layout& rhs) = delete;
        Layout& operator=(Layout&& rhs);

        VkPipelineLayout native() const;
    private:
        VkPipelineLayout layout;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_PIPELINE_LAYOUT_HPP