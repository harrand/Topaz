#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
#if TZ_VULKAN
#include "gl/impl/backend/vk2/image_format.hpp"
//#include "gl/impl/backend/vk2/swapchain.hpp"

namespace tz::gl::vk2
{
	class Swapchain;
	class LogicalDevice;

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies parameters of an Image referring to an existing Swapchain image.
	 */
	struct SwapchainImageInfo
	{
		/// Swapchain owning this image. Must not be null.
		const Swapchain* swapchain;
		/// Swapchains have a variable number of images. This is the index of the image collection owned by `swapchain`.
		std::uint32_t image_index;
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Represents an Image owned by the Vulkan API. This includes Swapchain images!
	 */
	class Image
	{
	public:
		/**
		 * Create an Image that refers to a Swapchain image.
		 * @param info Information about the Swapchain and which image to refer to.
		 */
		Image(SwapchainImageInfo sinfo);
		Image(const Image& copy) = delete;
		Image(Image&& move);
		~Image();

		Image& operator=(const Image& rhs) = delete;
		Image& operator=(Image&& rhs);
	private:
		VkImage image;
		const LogicalDevice* device;
		bool destroy_on_destructor;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
