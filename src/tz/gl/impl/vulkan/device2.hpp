#ifndef TZ_GL_IMPL_VULKAN_DEVICE_HPP
#define TZ_GL_IMPL_VULKAN_DEVICE_HPP
#if TZ_VULKAN
#include "tz/core/data/vector.hpp"
#include "tz/gl/api/device.hpp"
#include "tz/gl/impl/vulkan/renderer.hpp"
#include "tz/gl/impl/vulkan/detail/swapchain.hpp"
#include "tz/gl/impl/vulkan/detail/image.hpp"

namespace tz::gl
{
	class device_window
	{
	public:
		device_window(const vk2::LogicalDevice& device);
		const vk2::Swapchain& get_swapchain() const;
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

	class device_vulkan_base
	{
	public:
		device_vulkan_base();
		const vk2::LogicalDevice& vk_get_logical_device() const;
	protected:
		vk2::LogicalDevice ldev;
	};

	class device_vulkan2 : public device_common<renderer_vulkan>, public device_vulkan_base, public device_window
	{
	public:
		device_vulkan2();
	};
}

#endif // TZ_VULKAN
#endif // TZ_GL_IMPL_VULKAN_DEVICE_HPP
