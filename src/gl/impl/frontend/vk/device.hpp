#ifndef TOPAZ_GL_IMPL_VK_DEVICE_HPP
#define TOPAZ_GL_IMPL_VK_DEVICE_HPP
#if TZ_VULKAN
#include "gl/api/device.hpp"

#include "gl/impl/frontend/vk/render_pass.hpp"
#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/swapchain.hpp"
#include "gl/impl/backend/vk/pipeline/input_assembly.hpp"
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

	class DeviceWindowBufferVulkan : public std::variant<std::monostate, vk::Swapchain, vk::Image>
	{
	public:
		vk::Image::Format get_format() const;
		std::uint32_t get_width() const;
		std::uint32_t get_height() const;
		VkRect2D full_render_area() const;

		DeviceWindowBufferVulkan& operator=(vk::Swapchain&& rhs);
		DeviceWindowBufferVulkan& operator=(vk::Image&& rhs);
		DeviceWindowBufferVulkan& operator=(std::monostate rhs);
		explicit operator vk::Swapchain&();
		explicit operator const vk::Swapchain&() const;
		explicit operator vk::Image&();
		explicit operator const vk::Image&() const;
	private:
		using VariantType = std::variant<std::monostate, vk::Swapchain, vk::Image>;
	};

	class DeviceFunctionalityVulkan : public IDevice
	{
	public:
		RenderPassVulkan vk_create_render_pass(RenderPassBuilderVulkan builder) const;
		[[nodiscard]] virtual Renderer create_renderer(RendererBuilder builder) const final;
		[[nodiscard]] virtual Processor create_processor(ProcessorBuilderVulkan builder) const final;
		[[nodiscard]] virtual Shader create_shader(ShaderBuilder builder) const final;
		virtual TextureFormat get_window_format() const final;
	protected:
		DeviceFunctionalityVulkan();
		
		vk::hardware::Device physical_device;
		vk::LogicalDevice device;
		DeviceWindowBufferVulkan swapchain;
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

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_VK_DEVICE_HPP