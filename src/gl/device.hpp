#ifndef TOPAZ_GL_DEVICE_HPP
#define TOPAZ_GL_DEVICE_HPP
#include "gl/render_pass.hpp"

#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#elif TZ_OGL

#endif

namespace tz::gl
{
#if TZ_VULKAN
    class DeviceVulkan
    {
    public:
        DeviceVulkan();
        RenderPass create_render_pass(RenderPassBuilder builder) const;
    private:
        vk::hardware::Device physical_device;
        vk::LogicalDevice device;
    };

    using Device = DeviceVulkan;
#elif TZ_OGL
    class DeviceOGL
    {
    public:

    private:

    };
    
    using Device = DeviceOGL;
#endif
}

#endif // TOPAZ_GL_DEVICE_HPP