#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#if TZ_VULKAN
#include "core/callback.hpp"
#include "core/handle.hpp"
#include "gl/api/device.hpp"
#include "gl/impl/frontend/vk2/renderer.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"
#include "gl/impl/backend/vk2/image.hpp"

namespace tz::gl
{
	class DeviceWindowVulkan
	{
	public:
		DeviceWindowVulkan() = default;
		DeviceWindowVulkan(const vk2::LogicalDevice& device);
		DeviceWindowVulkan(const DeviceWindowVulkan& copy) = delete;
		DeviceWindowVulkan(DeviceWindowVulkan&& move);
		~DeviceWindowVulkan();
		DeviceWindowVulkan& operator=(const DeviceWindowVulkan& rhs) = delete;
		DeviceWindowVulkan& operator=(DeviceWindowVulkan&& rhs);

		bool valid() const;
		const vk2::Swapchain& get_swapchain() const;
		vk2::Swapchain& get_swapchain();

		tz::Vec2ui get_dimensions() const;
		vk2::ImageFormat get_format() const;

		vk2::Swapchain::ImageAcquisitionResult get_unused_image(const vk2::Swapchain::ImageAcquisition& acquire);
		bool has_unused_image() const;
		void mark_image_used();

		RendererResizeCallbackType& resize_callback();
		std::span<vk2::Image> get_output_images();
	private:
		void on_resize(tz::Vec2ui dims);
		void register_resize();
		void unregister_resize();
		bool is_resize_registered() const;
		void reregister_resize();

		vk2::Swapchain window_buf = vk2::Swapchain::null();
		tz::CallbackHandle on_resize_handle = tz::nullhand;
		RendererResizeCallbackType renderer_resize_callbacks = {};
		std::optional<vk2::Swapchain::ImageAcquisitionResult> recent_acquire = std::nullopt;
	};

	class DeviceRenderSchedulerVulkan
	{
	public:
		DeviceRenderSchedulerVulkan(const vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count);
		std::span<const vk2::BinarySemaphore> get_image_signals() const;
		std::span<const vk2::BinarySemaphore> get_render_work_signals() const;
		std::span<const vk2::Fence> get_frame_fences() const;
		void wait_frame_work_complete() const;
	private:
		std::vector<vk2::BinarySemaphore> image_available;
		std::vector<vk2::BinarySemaphore> render_work_done;
		std::vector<vk2::Fence> frame_work;
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
		static vk2::LogicalDevice make_device(const vk2::VulkanInstance& instance);

		vk2::LogicalDevice device;
		DeviceWindowVulkan window_storage;
		DeviceRenderSchedulerVulkan scheduler;
	};

	static_assert(DeviceType<DeviceVulkan, RendererInfoVulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
