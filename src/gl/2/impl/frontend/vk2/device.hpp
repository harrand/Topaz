#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#if TZ_VULKAN
#include "gl/2/api/device.hpp"
#include "gl/2/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"
#include "gl/impl/backend/vk2/image.hpp"

namespace tz::gl2
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

		// Satisfies DeviceType.
		RendererVulkan create_renderer(const RendererInfoVulkan& info);
		ImageFormat get_window_format() const;
	private:
		vk2::LogicalDevice device;
		DeviceWindowVulkan window_storage;
	};

	static_assert(DeviceType<DeviceVulkan, RendererInfoVulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
