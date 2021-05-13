#ifndef TOPAZ_GL_IMPL_VK_DEVICE_HPP
#define TOPAZ_GL_IMPL_VK_DEVICE_HPP
#include "gl/api/device.hpp"

#include "gl/vk/logical_device.hpp"
#include "gl/vk/swapchain.hpp"
#include "gl/vk/pipeline/input_assembly.hpp"

namespace tz::gl
{
    class DeviceBuilderVulkan
    {
    public:
        DeviceBuilderVulkan();
        /**
         * See @ref IDeviceBuilder::set_primitive_type(GraphicsPrimitiveType)
         */
        void set_primitive_type(GraphicsPrimitiveType type);
        /**
         * See @ref IDeviceBuilder::get_primitive_type()
         */
        GraphicsPrimitiveType get_primitive_type() const;
        
        vk::pipeline::PrimitiveTopology vk_get_primitive_topology() const;
    private:
        GraphicsPrimitiveType primitive_type;
    };

    class DeviceFunctionalityVulkan : public IDevice
    {
    public:
        virtual RenderPass create_render_pass(RenderPassBuilder builder) const final;
        virtual Renderer create_renderer(RendererBuilder builder) const final;
        virtual Shader create_shader(ShaderBuilder builder) const final;
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