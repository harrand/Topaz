#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
#if TZ_VULKAN
#include "hdk/data/vector.hpp"
#include "tz/core/containers/enum_field.hpp"
#include "tz/gl/impl/backend/vk2/debugname.hpp"
#include "tz/gl/impl/backend/vk2/image_format.hpp"
#include "tz/gl/impl/backend/vk2/gpu_mem.hpp"
#include "vk_mem_alloc.h"
#include <optional>
#include <string>

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
		TransferSource = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		/// - Only useable as a destination image of some transfer command.
		TransferDestination = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		/// - Useable as a presentable image.
		Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies the number of samples stored per image pixel.
	 */
	enum class SampleCount
	{
		One = VK_SAMPLE_COUNT_1_BIT,
		Two = VK_SAMPLE_COUNT_2_BIT,
		Four = VK_SAMPLE_COUNT_4_BIT,
		Eight = VK_SAMPLE_COUNT_8_BIT,
		Sixteen = VK_SAMPLE_COUNT_16_BIT,
		ThirtyTwo = VK_SAMPLE_COUNT_32_BIT,
		SixtyFour = VK_SAMPLE_COUNT_64_BIT
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies how the image is laid out in memory.
	 */
	enum class ImageTiling
	{
		/// Image texels are laid out in an implementation-defined manner.
		Optimal = VK_IMAGE_TILING_OPTIMAL,
		/// Image texels are laid out in memory in row-major order, possibly with some padding on each row.
		Linear = VK_IMAGE_TILING_LINEAR
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies intended usage of an @ref Image.
	 */
	enum class ImageUsage
	{
		/// - Image can be used as a source in a transfer command.
		TransferSource = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		/// - Image can be used as a destination in a transfer command.
		TransferDestination = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		/// - Image can be used as a read-only shader resource.
		SampledImage = VK_IMAGE_USAGE_SAMPLED_BIT,
		/// - Image can be used as a read/write shader resource.
		StorageImage = VK_IMAGE_USAGE_STORAGE_BIT,
		/// - Image is suitable as a colour attachment within a @ref Framebuffer.
		ColourAttachment = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		/// - Image is suitable as a depth/stencil attachment within a @ref Framebuffer.
		DepthStencilAttachment = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		/// - Image is suitable as an input attachment within a @ref Framebuffer.
		InputAttachment = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
	};

	using ImageUsageField = tz::EnumField<ImageUsage>;

	/**
	 * @ingroup tz_gl_vk_image
	 * Specifies which aspects of the image are included within a view.
	 */
	enum class ImageAspectFlag
	{
		Colour = VK_IMAGE_ASPECT_COLOR_BIT,
		Depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		Stencil = VK_IMAGE_ASPECT_STENCIL_BIT
	};

	using ImageAspectFlags = tz::EnumField<ImageAspectFlag>;

	constexpr ImageAspectFlags derive_aspect_from_format(ImageFormat fmt)
	{
		switch(fmt)
		{
			case ImageFormat::Depth16_UNorm:
			[[fallthrough]];
			case ImageFormat::Depth32_SFloat:
				return {ImageAspectFlag::Depth};
			break;
			default:
				return {ImageAspectFlag::Colour};
			break;
		}
	}


	class Swapchain;
	class LogicalDevice;
	namespace hardware
	{
		class Queue;
	}

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
	 * Specifies creation flags for an @ref Image.
	 */
	struct ImageInfo
	{
		/// LogicalDevice owning this image. Must not be null.
		const LogicalDevice* device;
		/// Format of the image.
		ImageFormat format;
		/// Dimensions of the image, in pixels.
		hdk::vec2ui dimensions;
		/// Field of expected usages of the image. For example, if you wish to transition the image to @ref ImageLayout::TransferTo then this usage field must contain @ref ImageUsage::TransferDestination.
		ImageUsageField usage;
		/// Describes where the image is laid out in memory.
		MemoryResidency residency;
		/// Specifies how many mip levels there are. Default 1.
		std::uint32_t mip_levels = 1;
		/// Specifies how many layers there are. Default 1.
		std::uint32_t array_layers = 1;
		/// Specifies how many times the image is sampled per pixel. Default 1.
		SampleCount sample_count = SampleCount::One;
		/// Specifies image tiling. Default ImageTiling::Optimal.
		ImageTiling image_tiling = ImageTiling::Optimal;
	};

	/**
	 * @ingroup tz_gl_vk_image
	 * Represents an Image owned by the Vulkan API. This includes Swapchain images!
	 */
	class Image : public DebugNameable<VK_OBJECT_TYPE_IMAGE>
	{
	public:
		/**
		 * Create an Image that refers to a Swapchain image.
		 * @param info Information about the Swapchain and which image to refer to.
		 */
		Image(SwapchainImageInfo sinfo);
		Image(ImageInfo info);
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
		/**
		 * Retrieve the dimensions of the image.
		 * @return {width, height} of the image, in pixels.
		 */
		hdk::vec2ui get_dimensions() const;
		/**
		 * Retrieve the @ref LogicalDevice that 'owns' the image.
		 *
		 * Swapchain images are owned by the presentation device. For that reason, these do not belong to this device per-se. In this case this will return the @ref LogicalDevice responsible for retrieving the image (Most certainly the LogicalDevice that initially spawned its owner Swapchain.)
		 */
		const LogicalDevice& get_device() const;

		void* map();

		template<typename T>
		std::span<T> map_as()
		{
			auto* ptr = reinterpret_cast<T*>(this->map());
			if(ptr == nullptr)
			{
				return {ptr, 0};
			}
			return {ptr, this->vma_alloc_info.size / sizeof(T)};
		}

		void unmap();
		std::size_t get_linear_row_length() const;

		using NativeType = VkImage;
		NativeType native() const;
		static Image null();
		bool is_null() const;

		bool operator==(const Image& rhs) const;

		friend class hardware::Queue;
	private:
		Image();
		void set_layout(ImageLayout layout);

		VkImage image;
		ImageFormat format;
		ImageLayout layout;
		ImageTiling tiling;
		MemoryResidency residency;
		hdk::vec2ui dimensions;
		const LogicalDevice* device;
		bool destroy_on_destructor;
		std::optional<VmaAllocation> vma_alloc;
		VmaAllocationInfo vma_alloc_info;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_HPP
