#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#if TZ_VULKAN
#include "hdk/data/handle.hpp"
#include "tz/core/callback.hpp"
#include "tz/gl/api/device.hpp"
#include "tz/gl/impl/frontend/common/device.hpp"
#include "tz/gl/impl/frontend/vk2/renderer.hpp"
#include "tz/gl/impl/backend/vk2/swapchain.hpp"
#include "tz/gl/impl/backend/vk2/image.hpp"

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

		hdk::vec2ui get_dimensions() const;
		vk2::ImageFormat get_format() const;

		vk2::Swapchain::ImageAcquisitionResult get_unused_image(const vk2::Swapchain::ImageAcquisition& acquire);
		bool has_unused_image() const;
		void mark_image_used();

		RendererResizeCallbackType& resize_callback();
		std::span<vk2::Image> get_output_images();
		vk2::Image& get_depth_image();
	private:
		void set_swapchain_images_debug_name();
		void make_depth_image();
		void on_resize(hdk::vec2ui dims);
		void register_resize();
		void unregister_resize();
		bool is_resize_registered() const;
		void reregister_resize();

		vk2::Swapchain window_buf = vk2::Swapchain::null();
		vk2::Image old_depth_image = vk2::Image::null();
		vk2::Image depth_image = vk2::Image::null();
		tz::CallbackHandle on_resize_handle = hdk::nullhand;
		RendererResizeCallbackType renderer_resize_callbacks = {};
		std::optional<vk2::Swapchain::ImageAcquisitionResult> recent_acquire = std::nullopt;
	};

	class DeviceRenderSchedulerVulkan
	{
	public:
		DeviceRenderSchedulerVulkan(const vk2::LogicalDevice& ldev, std::size_t frame_in_flight_count);
		void notify_renderer_added();
		void notify_renderer_removed(std::size_t renderer_id);
		std::span<const vk2::BinarySemaphore> get_image_signals() const;
		std::span<vk2::BinarySemaphore> get_image_signals();
		std::span<const vk2::BinarySemaphore> get_render_work_signals() const;
		std::span<const vk2::Fence> get_frame_fences() const;
		std::span<vk2::Fence> get_frame_fences();
		std::span<const vk2::TimelineSemaphore> get_renderer_timelines() const;
		std::span<vk2::TimelineSemaphore> get_renderer_timelines();
		void wait_frame_work_complete() const;
	private:
		const vk2::LogicalDevice* ldev;
		std::vector<vk2::BinarySemaphore> image_available;
		std::vector<vk2::BinarySemaphore> render_work_done;
		std::vector<vk2::Fence> frame_work;
		std::vector<vk2::TimelineSemaphore> renderer_timelines;
	};

	class DeviceVulkan : public DeviceCommon<RendererVulkan>
	{
	public:
		DeviceVulkan();
		DeviceVulkan(const vk2::VulkanInstance& instance);
		DeviceVulkan(const DeviceVulkan& copy) = delete;
		~DeviceVulkan();

		// Satisfies DeviceType.
		tz::gl::RendererHandle create_renderer(const RendererInfoVulkan& info);
		using DeviceCommon<RendererVulkan>::get_renderer;
		void destroy_renderer(tz::gl::RendererHandle handle);
		ImageFormat get_window_format() const;
		void dbgui();
		const vk2::LogicalDevice& vk_get_logical_device() const;
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
