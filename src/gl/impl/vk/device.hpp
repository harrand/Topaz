#ifndef TOPAZ_GL_IMPL_VK_DEVICE_HPP
#define TOPAZ_GL_IMPL_VK_DEVICE_HPP
#include "gl/render_pass.hpp"
#include "gl/renderer.hpp"
#include "gl/impl/common/device.hpp"

#include "gl/vk/logical_device.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/pipeline/input_assembly.hpp"

namespace tz::gl
{
    class DeviceBuilderVulkan
    {
    public:
        DeviceBuilderVulkan();
        void set_primitive_type(GraphicsPrimitiveType type);
        GraphicsPrimitiveType get_primitive_type() const;
        
        vk::pipeline::PrimitiveTopology vk_get_primitive_topology() const;
    private:
        GraphicsPrimitiveType primitive_type;
    };

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
        vk::pipeline::PrimitiveTopology primitive_type;
    };

    class DeviceVulkan : public DeviceFunctionalityVulkan
    {
    public:
        DeviceVulkan(DeviceBuilderVulkan builder);
    private:
        
    };
}

#endif // TOPAZ_GL_IMPL_VK_DEVICE_HPP