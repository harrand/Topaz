#ifndef TOPAZ_GL_IMPL_VK_DEVICE_HPP
#define TOPAZ_GL_IMPL_VK_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"

#include "gl/vk/logical_device.hpp"
#include "gl/vk/swapchain.hpp"

namespace tz::gl
{
    class DeviceFunctionalityVulkan
    {
    public:
        RenderPass create_render_pass(RenderPassBuilder builder) const;
        Renderer create_renderer(RendererBuilder builder) const;
    protected:
        DeviceFunctionalityVulkan();
        
        vk::hardware::Device physical_device;
        vk::LogicalDevice device;
        vk::Swapchain swapchain;
    };

    class DeviceVulkan : public DeviceFunctionalityVulkan
    {
    public:
        DeviceVulkan();
    };
}

#endif // TOPAZ_GL_IMPL_VK_DEVICE_HPP