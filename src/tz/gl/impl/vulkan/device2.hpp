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
#include <deque>

namespace tz::gl
{
	class device_vulkan_base
	{
	public:
		device_vulkan_base();
		const vk2::LogicalDevice& vk_get_logical_device() const;
		vk2::LogicalDevice& vk_get_logical_device();
		std::size_t vk_get_frame_id() const{return this->frame_id;}
		std::size_t get_rid(unsigned int fingerprint) const;
	protected:
		void touch_renderer_id(unsigned int fingerprint, std::size_t renderer_id);

		std::size_t frame_id = 0;
		std::size_t old_frame_id = 0;
		std::size_t frame_counter = 0;
		std::size_t global_timeline = 0;
		std::size_t max_signal_rank_this_frame = 0;
		bool frame_signal_sent_this_frame = false;
		std::unordered_map<unsigned int, std::size_t> fingerprint_to_renderer_id = {};
		vk2::LogicalDevice ldev;
	};

	class device_window : public device_vulkan_base
	{
	public:
		device_window();
		tz::gl::image_format get_window_format() const;
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
		const vk2::BinarySemaphore& acquire_image(const vk2::Fence* signal_fence);
		std::size_t get_image_index() const;
		std::span<vk2::BinarySemaphore> get_image_semaphores();
		const vk2::Swapchain::ImageAcquisitionResult* get_recent_acquire() const;
		bool is_vsync_enabled_impl() const;
		void set_vsync_enabled_impl(bool vsync);
		void vk_acquire_done();
		void vk_notify_resize();
	private:
		void make_depth_image();
		void initialise_image_semaphores();
		void debug_annotate_resources();
		vk2::Swapchain swapchain = vk2::Swapchain::null();
		vk2::Image device_depth = vk2::Image::null();
		tz::vec2ui dimensions_cache = tz::vec2ui::zero();
		std::optional<vk2::Swapchain::ImageAcquisitionResult> recent_acquire = std::nullopt;
		std::vector<vk2::BinarySemaphore> image_semaphores = {};
	};

	class device_render_sync : public device_window
	{
	public:
		device_render_sync(device_common<renderer_vulkan2>& devcom);
		void vk_frame_wait(unsigned int fingerprint);
		void vk_skip_renderer(unsigned int fingerprint);
		const tz::gl::schedule& get_schedule() const;
		const tz::gl::timeline_t& get_timeline() const;
		std::vector<const vk2::Semaphore*> vk_get_dependency_waits(unsigned int fingerprint);
		std::vector<const vk2::Semaphore*> vk_get_dependency_signals(unsigned int fingerprint);
	protected:
		std::span<const vk2::TimelineSemaphore> get_frame_sync_objects() const;
		std::span<vk2::TimelineSemaphore> get_frame_sync_objects();
		std::span<vk2::TimelineSemaphore> get_dependency_sync_objects();
		std::uint64_t get_sync_id(std::size_t renderer_id) const;
		std::uint64_t get_renderer_sync_id(unsigned int fingerprint) const;
	private:
		const tz::gl::schedule& sched;
		std::vector<vk2::TimelineSemaphore> dependency_syncs = {};
		std::vector<vk2::TimelineSemaphore> frame_syncs = {};
	};

	class device_descriptor_pool : public device_render_sync
	{
	public:
		device_descriptor_pool(device_common<renderer_vulkan2>& devcom);
		vk2::DescriptorPool::UpdateRequest vk_make_update_request(unsigned int fingerprint);
		vk2::DescriptorPool::AllocationResult vk_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint);
		void vk_update_sets(vk2::DescriptorPool::UpdateRequest update, unsigned int fingerprint);
	private:
		vk2::DescriptorPool& get_pool(unsigned int fingerprint);
		vk2::DescriptorPool::AllocationResult impl_allocate_sets(const vk2::DescriptorPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt);
		void another_pool();
		void another_pool(std::size_t set_count, std::size_t buf_count, std::size_t img_count);
		std::deque<vk2::DescriptorPool> pools = {};
		std::unordered_map<unsigned int, std::size_t> fingerprint_to_pool_id = {};
	};

	class device_command_pool : public device_descriptor_pool
	{
	public:
		struct fingerprint_info_t
		{
			bool compute = false;
			bool requires_present = true;
		};
		device_command_pool(device_common<renderer_vulkan2>& devcom);
		vk2::CommandPool::AllocationResult vk_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint);
		void vk_free_commands(unsigned int fingerprint, std::size_t allocation_id, std::span<vk2::CommandBuffer> command_buffers);
		void vk_command_pool_touch(unsigned int fingerprint, fingerprint_info_t finfo);
		void vk_submit_and_run_commands_blocking(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, const vk2::CommandBuffer& buffer);
		void vk_submit_command(unsigned int fingerprint, std::size_t allocation_id, std::size_t buffer_id, std::span<const vk2::CommandBuffer> cmdbufs, const tz::basic_list<const vk2::BinarySemaphore*>& extra_waits, const tz::basic_list<const vk2::BinarySemaphore*>& extra_signals, vk2::Fence* signal_fence);
		/**
		* present the acquired image.
		*
		* @pre @ref acquire_image() should have been invoked earlier, in such a way that the acquisition will have been completed by the time the present command is ran GPU-side
		*
		*/
		vk2::hardware::Queue::PresentResult present_image(unsigned int fingerprint, const tz::basic_list<const vk2::BinarySemaphore*>& wait_semaphores);
	private:
		struct allocation_history
		{
			fingerprint_info_t info;
			vk2::CommandPool::Allocation alloc;
		};
		vk2::CommandPool::AllocationResult impl_allocate_commands(const vk2::CommandPool::Allocation& alloc, unsigned int fingerprint, unsigned int attempt);
		vk2::CommandPool& get_fitting_pool(const fingerprint_info_t& finfo);
		vk2::hardware::Queue* get_original_queue(const fingerprint_info_t& finfo);
		vk2::hardware::Queue* graphics_queue = nullptr;
		vk2::hardware::Queue* graphics_present_queue = nullptr;
		vk2::hardware::Queue* compute_queue = nullptr;
		vk2::CommandPool graphics_commands = vk2::CommandPool::null();
		vk2::CommandPool graphics_present_commands = vk2::CommandPool::null();
		vk2::CommandPool compute_commands = vk2::CommandPool::null();
		std::unordered_map<unsigned int, fingerprint_info_t> fingerprint_alloc_types = {};
		std::unordered_map<unsigned int, std::vector<vk2::CommandPool::Allocation>> fingerprint_allocation_history = {};
	};

	class device_vulkan2 :
		public device_common<renderer_vulkan2>,
		public device_command_pool
	{
	public:
		device_vulkan2();
		~device_vulkan2();
		tz::gl::renderer_handle create_renderer(const tz::gl::renderer_info& rinfo);
		bool is_vsync_enabled() const;
		void set_vsync_enabled(bool vsync);
		using device_common<renderer_vulkan2>::get_renderer;
		void dbgui();
		void begin_frame(){}
		void end_frame();
		void full_wait() const;
		void frame_wait() const;
	};
	static_assert(device_type<device_vulkan2, renderer_info>);
}

#endif // TZ_VULKAN
#endif // TZ_GL_IMPL_VULKAN_DEVICE_HPP
