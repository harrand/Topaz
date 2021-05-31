#ifndef TOPAZ_GL_IMPL_VK_DEVICE_HPP
#define TOPAZ_GL_IMPL_VK_DEVICE_HPP
#include "gl/api/device.hpp"

#include "gl/vk/logical_device.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/pipeline/input_assembly.hpp"
#include <deque>

namespace tz::gl
{
    class DeviceBuilderVulkan : public IDeviceBuilder
    {
    public:
        DeviceBuilderVulkan();
        virtual void set_primitive_type(GraphicsPrimitiveType type) final;
        virtual GraphicsPrimitiveType get_primitive_type() const final;
        
        vk::pipeline::PrimitiveTopology vk_get_primitive_topology() const;
    private:
        GraphicsPrimitiveType primitive_type;
    };

    class DeviceFunctionalityVulkan : public IDevice
    {
    public:
        [[nodiscard]] virtual RenderPass create_render_pass(RenderPassBuilder builder) const final;
        [[nodiscard]] virtual Renderer create_renderer(RendererBuilder builder) const final;
        [[nodiscard]] virtual Shader create_shader(ShaderBuilder builder) const final;
    protected:
        DeviceFunctionalityVulkan();
        
        vk::hardware::Device physical_device;
        vk::LogicalDevice device;
        vk::Swapchain swapchain;
        vk::pipeline::PrimitiveTopology primitive_type;
    private:
        void on_window_resize();

        mutable std::deque<DeviceWindowResizeCallback> renderer_resize_callbacks;
    };

    class DeviceVulkan : public DeviceFunctionalityVulkan
    {
    public:
        DeviceVulkan(DeviceBuilderVulkan builder);
    private:
        
    };
}

#endif // TOPAZ_GL_IMPL_VK_DEVICE_HPP