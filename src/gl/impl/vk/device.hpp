#ifndef TOPAZ_GL_IMPL_VK_DEVICE_HPP
#define TOPAZ_GL_IMPL_VK_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/vk/logical_device.hpp"

namespace tz::gl
{
    class DeviceVulkan
    {
    public:
        DeviceVulkan();
        RenderPass create_render_pass(RenderPassBuilder builder) const;
    private:
        vk::hardware::Device physical_device;
        vk::LogicalDevice device;
    };
}

#endif // TOPAZ_GL_IMPL_VK_DEVICE_HPP