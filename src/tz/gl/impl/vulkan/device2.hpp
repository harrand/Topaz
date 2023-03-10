#ifndef TZ_GL_IMPL_VULKAN_DEVICE_HPP
#define TZ_GL_IMPL_VULKAN_DEVICE_HPP
#if TZ_VULKAN
#include "tz/core/data/vector.hpp"
#include "tz/gl/api/device.hpp"
#include "tz/gl/impl/vulkan/renderer2.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"
#include "tz/gl/impl/vulkan/detail/image.hpp"
#include "tz/gl/impl/vulkan/detail/semaphore.hpp"
#include "tz/gl/impl/vulkan/detail/descriptors.hpp"
#include "tz/gl/impl/vulkan/detail/command.hpp"
#include <unordered_map>

namespace tz::gl
{
	class device_window
	{
	public:
		device_window(const vk2::LogicalDevice& device);
		const vk2::Swapchain& get_swapchain() const;
		vk2::Swapchain& get_swapchain();
		const vk2::Image& get_depth_image() const;
		vk2::Image& get_depth_image();
		/**
		 * acquire an image from the underlying presentation engine. if you want to render something to the window, you will need to acquire an image here and render into it.
		 * if you acquired an image earlier but didn't present to it, this will give you the same image.
		 *
		 * this method returns instantly. to schedule work to occur after the acquisition completes, fill in the `acquire` parameter which has a fence and/or semaphore you can wait on after.
		 *
		 * @return swapchain index image that will be retrieved.
		 */
		std::size_t acquire_image(const vk2::Swapchain::ImageAcquisition& acquire);
		/**
		* present the acquired image.
		*
		* @pre @ref acquire_image() should have been invoked earlier, in such a way that the acquisition will have been completed by the time the present command is ran GPU-side
		*
		*/
		vk2::hardware::Queue::PresentResult present_image(vk2::hardware::Queue& present_queue, std::span<const vk2::BinarySemaphore> wait_semaphores);
	private:
		void make_depth_image();
		void debug_annotate_resources();
		const vk2::LogicalDevice* ldev = nullptr;
		vk2::Swapchain swapchain = vk2::Swapchain::null();
		vk2::Image device_depth = vk2::Image::null();
		tz::vec2ui dimensions_cache = tz::vec2ui::zero();
		std::optional<vk2::Swapchain::ImageAcquisitionResult> recent_acquire = std::nullopt;
	};

	class device_render_sync
	{
	public:
		device_render_sync(const vk2::LogicalDevice& ldev, const tz::gl::timeline_t& timeline, std::size_t frame_in_flight_count);
		void vk_frame_wait(unsigned int fingerprint);
		std::vector<const vk2::Semaphore*> vk_get_dependency_waits(unsigned int fingerprint);
		std::vector<const vk2::Semaphore*> vk_get_dependency_signals(unsigned int fingerprint);
	protected:
		std::size_t get_rid(unsigned int fingerprint) const;
		void touch_renderer_id(unsigned int fingerprint, std::size_t renderer_id);
	private:
		const tz::gl::timeline_t& timeline;
		vk2::TimelineSemaphore tsem;
		std::unordered_map<unsigned int, std::size_t> fingerprint_to_renderer_id = {};
		std::vector<vk2::Fence> frame_fences = {};
		std::size_t frame_id = 0;
	};

	class device_descriptor_pool
	{
	public:
		device_descriptor_pool(const vk2::LogicalDevice& device);
		vk2::DescriptorPool::UpdateRequest vk_make_update_request(unsigned int fingerprint);
		vk2::DescriptorPool::AllocationResult vk_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint);
		void vk_update_sets(vk2::DescriptorPool::UpdateRequest update, unsigned int fingerprint);
	private:
		vk2::DescriptorPool& get_pool(unsigned int fingerprint);
		vk2::DescriptorPool::AllocationResult impl_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt);
		void another_pool();
		void another_pool(std::size_t set_count, std::size_t buf_count, std::size_t img_count);
		const vk2::LogicalDevice* ldev;
		std::vector<vk2::DescriptorPool> pools = {};
		std::unordered_map<unsigned int, std::size_t> fingerprint_to_pool_id = {};
	};

	class device_command_pool
	{
	public:
		struct fingerprint_info_t
		{
			bool compute = false;
			bool requires_present = true;
		};
		device_command_pool(vk2::LogicalDevice& device);
		vk2::CommandPool::AllocationResult vk_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint);
		void vk_free_commands(unsigned int fingerprint, std::size_t allocation_id, std::span<vk2::CommandBuffer> command_buffers);
		void vk_command_pool_touch(unsigned int fingerprint, fingerprint_info_t finfo);
		void vk_submit_and_run_commands_blocking(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, const vk2::CommandBuffer& buffer);
		void vk_submit_command(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, vk2::hardware::Queue::SubmitInfo submit);
	private:
		struct allocation_history
		{
			fingerprint_info_t info;
			vk2::CommandPool::Allocation alloc;
		};
		vk2::CommandPool::AllocationResult impl_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt);
		vk2::CommandPool& get_fitting_pool(const fingerprint_info_t& finfo);
		vk2::hardware::Queue* get_original_queue(const fingerprint_info_t& finfo);
		vk2::LogicalDevice* ldev;
		vk2::hardware::Queue* graphics_queue = nullptr;
		vk2::hardware::Queue* graphics_present_queue = nullptr;
		vk2::hardware::Queue* compute_queue = nullptr;
		vk2::CommandPool graphics_commands = vk2::CommandPool::null();
		vk2::CommandPool graphics_present_commands = vk2::CommandPool::null();
		vk2::CommandPool compute_commands = vk2::CommandPool::null();
		std::unordered_map<unsigned int, fingerprint_info_t> fingerprint_alloc_types = {};
		std::unordered_map<unsigned int, std::vector<vk2::CommandPool::Allocation>> fingerprint_allocation_history = {};
	};

	class device_vulkan_base
	{
	public:
		device_vulkan_base();
		const vk2::LogicalDevice& vk_get_logical_device() const;
		vk2::LogicalDevice& vk_get_logical_device();
	protected:
		vk2::LogicalDevice ldev;
	};

	class device_vulkan2 :
		public device_common<renderer_vulkan2>,
		public device_vulkan_base,
		public device_window,
		public device_render_sync,
		public device_descriptor_pool,
		public device_command_pool
	{
	public:
		device_vulkan2();
		tz::gl::renderer_handle create_renderer(const tz::gl::renderer_info& rinfo);
		using device_common<renderer_vulkan2>::get_renderer;
		image_format get_window_format() const;
		void dbgui(){}
		void begin_frame(){}
		void end_frame(){}
	};
	static_assert(device_type<device_vulkan2, renderer_info>);
}

#endif // TZ_VULKAN
#endif // TZ_GL_IMPL_VULKAN_DEVICE_HPP
