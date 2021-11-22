#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
#if TZ_VULKAN
#include "core/vector.hpp"
#include "gl/impl/backend/vk2/image_format.hpp"

namespace tz::gl::vk2
{
	/**
	 * @ingroup tz_gl_vk_image
	 * Images are always in a layout. Images can only perform certain operations in a given layout.
	 */
	enum class ImageLayout
	{
		/// - Cannot be used for most operations. If used instead of the image's true layout in a layout transition, the contents of the image's memory will become undefined.
		Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
		/// - Supports all operations, however is unlikely to perform optimally for any such operations.
		General = VK_IMAGE_LAYOUT_GENERAL,
		/// - Only useable as a colour/resolve attachment within a @ref Framebuffer.
		ColourAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		/// - Only useable as a depth/stencil attachment.
		DepthStencilAttachment = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		/// - Read-only access in a @ref Shader as a sampled image.
		ShaderResource = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		/// - Only useable as a source image of some transfer command.
		TransferFrom = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		/// - Only useable as a destination image of some transfer command.
		TransferTo = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		/// - Useable as a presentable image.
		Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

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

		/**
		 * Retrieve the underlying format of the image.
		 */
		ImageFormat get_format() const;
		/**
		 * Retrieve the current layout of the image.
		 */
		ImageLayout get_layout() const;
		Vec2ui get_dimensions() const;
		/**
		 * Retrieve the @ref LogicalDevice that 'owns' the image.
		 *
		 * Swapchain images are owned by the presentation device. For that reason, these do not belong to this device per-se. In this case this will return the @ref LogicalDevice responsible for retrieving the image (Most certainly the LogicalDevice that initially spawned its owner Swapchain.)
		 */
		const LogicalDevice& get_device() const;

		using NativeType = VkImage;
		NativeType native() const;
		static Image null();
		bool is_null() const;

		bool operator==(const Image& rhs) const;
	private:
		Image();

		VkImage image;
		ImageFormat format;
		ImageLayout layout;
		Vec2ui dimensions;
		const LogicalDevice* device;
		bool destroy_on_destructor;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
