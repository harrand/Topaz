#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#include "core/callback.hpp"
#include "core/handle.hpp"
#if TZ_VULKAN
#include "gl/api/device.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"
#include "gl/impl/backend/vk2/image.hpp"

namespace tz::gl2
{
	class DeviceWindowVulkan
	{
	public:
		using ResizeCallbackType = tz::WindowFunctionality::ResizeCallbackType;

		DeviceWindowVulkan() = default;
		DeviceWindowVulkan(const vk2::LogicalDevice& device);
		DeviceWindowVulkan(const DeviceWindowVulkan& copy) = delete;
		DeviceWindowVulkan(DeviceWindowVulkan&& move);
		~DeviceWindowVulkan();
		DeviceWindowVulkan& operator=(const DeviceWindowVulkan& rhs) = delete;
		DeviceWindowVulkan& operator=(DeviceWindowVulkan&& rhs);

		bool valid() const;
		vk2::Swapchain* as_swapchain();
		const vk2::Swapchain* as_swapchain() const;
		vk2::Image* as_image();
		const vk2::Image* as_image() const;

		tz::Vec2ui get_dimensions() const;
		vk2::ImageFormat get_format() const;

		ResizeCallbackType& resize_callback();
	private:
		void on_resize(tz::Vec2ui dims);
		void register_resize();
		void unregister_resize();
		bool is_resize_registered() const;
		void reregister_resize();

		std::variant<vk2::Swapchain, vk2::Image, std::monostate> window_buf = std::monostate{};
		tz::CallbackHandle on_resize_handle = tz::nullhand;
		ResizeCallbackType renderer_resize_callbacks = {};
	};

	class DeviceVulkan
	{
	public:
		DeviceVulkan();
		DeviceVulkan(const vk2::VulkanInstance& instance);
		DeviceVulkan(const DeviceVulkan& copy) = delete;

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
