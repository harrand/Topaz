#ifndef TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#define TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
#if TZ_VULKAN
#include "hdk/data/handle.hpp"
#include "tz/core/callback.hpp"
#include "tz/gl/api/device.hpp"
#include "tz/gl/impl/common/device.hpp"
#include "tz/gl/impl/vulkan/renderer.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"
#include "tz/gl/impl/vulkan/detail/image.hpp"

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
		vk2::image_format get_format() const;

		vk2::Swapchain::ImageAcquisitionResult get_unused_image(const vk2::Swapchain::ImageAcquisition& acquire);
		bool has_unused_image() const;
		void mark_image_used();

		std::span<vk2::Image> get_output_images();
		vk2::Image& get_depth_image();
		bool request_refresh();
	private:
		void set_swapchain_images_debug_name();
		void make_depth_image();
		void on_resize(hdk::vec2ui dims);

		vk2::Swapchain window_buf = vk2::Swapchain::null();
		vk2::Image old_depth_image = vk2::Image::null();
		vk2::Image depth_image = vk2::Image::null();
		std::optional<vk2::Swapchain::ImageAcquisitionResult> recent_acquire = std::nullopt;
		hdk::vec2ui window_dims_cache = {};
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
		void clear_renderers();
	private:
		const vk2::LogicalDevice* ldev;
		std::vector<vk2::BinarySemaphore> image_available;
		std::vector<vk2::BinarySemaphore> render_work_done;
		std::vector<vk2::Fence> frame_work;
		std::vector<vk2::TimelineSemaphore> renderer_timelines;
	};

	class device_vulkan : public DeviceCommon<renderer_vulkan>
	{
	public:
		device_vulkan();
		device_vulkan(const vk2::VulkanInstance& instance);
		device_vulkan(const device_vulkan& copy) = delete;
		~device_vulkan();

		// Satisfies device_type.
		tz::gl::renderer_handle create_renderer(const renderer_info_vulkan& info);
		using DeviceCommon<renderer_vulkan>::get_renderer;
		void destroy_renderer(tz::gl::renderer_handle handle);
		image_format get_window_format() const;
		void dbgui();
		const DeviceWindowVulkan& get_device_window() const;
		DeviceWindowVulkan& get_device_window();
		const DeviceRenderSchedulerVulkan& get_render_scheduler() const;
		DeviceRenderSchedulerVulkan& get_render_scheduler();
		const vk2::LogicalDevice& vk_get_logical_device() const;
		vk2::LogicalDevice& vk_get_logical_device();
	private:
		static vk2::LogicalDevice make_device(const vk2::VulkanInstance& instance);

		vk2::LogicalDevice device;
		DeviceWindowVulkan window_storage;
		DeviceRenderSchedulerVulkan scheduler;
	};

	static_assert(device_type<device_vulkan, renderer_info_vulkan>);
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL2_IMPL_FRONTEND_VK2_DEVICE_HPP
