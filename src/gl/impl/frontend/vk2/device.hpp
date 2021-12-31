#ifndef TOPAZ_GL_IMPL_FRONTEND_VK2_DEVICE_HPP
#define TOPAZ_GL_IMPL_FRONTEND_VK2_DEVICE_HPP
#if TZ_VULKAN
#include "gl/api/device.hpp"

#include "gl/impl/backend/vk2/swapchain.hpp"
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/frontend/vk2/shader.hpp"

namespace tz::gl
{
	class DeviceWindowVulkan
	{
	public:
		DeviceWindowVulkan();
		DeviceWindowVulkan(const vk2::LogicalDevice& device);
		bool valid() const;
		vk2::Swapchain* as_swapchain();
		const vk2::Swapchain* as_swapchain() const;
		vk2::Image* as_image();
		const vk2::Image* as_image() const;

		tz::Vec2ui get_dimensions() const;
		vk2::ImageFormat get_format() const;
	private:
		std::variant<vk2::Swapchain, vk2::Image, std::monostate> window_buf;
	};

	class DeviceVulkan
	{
	public:
		DeviceVulkan();
		DeviceVulkan(const vk2::VulkanInstance& instance);

		RendererVulkan2 create_renderer(const RendererBuilderVulkan2& builder);
		ShaderVulkan2 create_shader(const ShaderBuilderVulkan2& builder);
	private:
		vk2::LogicalDevice device;
		DeviceWindowVulkan window_storage;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_FRONTEND_VK2_DEVICE_HPP
