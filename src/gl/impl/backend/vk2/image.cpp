#if TZ_VULKAN
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	Image::Image(SwapchainImageInfo sinfo):
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	dimensions(),
	device(nullptr),
	destroy_on_destructor(false),
	vma_alloc(std::nullopt)
	{
		tz_assert(sinfo.swapchain != nullptr && !sinfo.swapchain->is_null(), "SwapchainImageInfo had nullptr or null Swapchain");
		std::uint32_t real_swapchain_image_count;
		std::vector<VkImage> swapchain_image_natives;

		vkGetSwapchainImagesKHR(sinfo.swapchain->get_device().native(), sinfo.swapchain->native(), &real_swapchain_image_count, nullptr);
		swapchain_image_natives.resize(real_swapchain_image_count);
		vkGetSwapchainImagesKHR(sinfo.swapchain->get_device().native(), sinfo.swapchain->native(), &real_swapchain_image_count, swapchain_image_natives.data());

		tz_assert(std::cmp_less(sinfo.image_index, real_swapchain_image_count), "SwapchainImageInfo image index %u is out of range of the Swapchain's true number of images (%u).", sinfo.image_index, real_swapchain_image_count);
		
		// Now actually initialise the object internals.
		this->image = swapchain_image_natives[sinfo.image_index];
		this->format = sinfo.swapchain->get_image_format();
		this->layout = ImageLayout::Undefined;
		this->dimensions = sinfo.swapchain->get_dimensions();
		this->device = &sinfo.swapchain->get_device();
		// I verified in the spec here: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#_wsi_swapchain that the initial layout of a swapchain image is guaranteed to be undefined.
	}

	Image::Image(ImageInfo info):
	image(VK_NULL_HANDLE),
	format(info.format),
	layout(ImageLayout::Undefined),
	dimensions(info.dimensions),
	device(info.device),
	destroy_on_destructor(true),
	vma_alloc(VmaAllocation{})
	{
		VkImageCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = static_cast<VkFormat>(this->format),
			.extent = {.width = info.dimensions[0], .height = info.dimensions[0], .depth = 1},
			.mipLevels = info.mip_levels,
			.arrayLayers = info.array_layers,
			.samples = static_cast<VkSampleCountFlagBits>(info.sample_count),
			.tiling = static_cast<VkImageTiling>(info.image_tiling),
			.usage = static_cast<VkImageUsageFlags>(static_cast<ImageUsage>(info.usage)),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = static_cast<VkImageLayout>(this->layout)
		};

		VmaMemoryUsage vma_usage;
		VmaAllocationCreateFlags vma_flags = 0;
		switch(info.residency)
		{
			case MemoryResidency::GPU:
				vma_usage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
			case MemoryResidency::CPU:
				vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
			break;
			case MemoryResidency::CPUPersistent:
				vma_usage = VMA_MEMORY_USAGE_CPU_ONLY;
				vma_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
			break;
			default:
				tz_error("Unrecognised MemoryResidency. Please submit a bug report.");
			break;
		}

		VmaAllocationCreateInfo alloc
		{
			.flags = vma_flags,
			.usage = vma_usage,
			.requiredFlags = 0,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool = {},
			.pUserData = nullptr,
			.priority = 0
		};

		VkResult res = vmaCreateImage(this->get_device().vma_native(), &create, &alloc, &this->image, &this->vma_alloc.value(), nullptr);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create Image because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create Image because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create Image but cannot determine why. Please submit a bug report.");
			break;
		}
	}

	Image::Image(Image&& move):
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	device(nullptr),
	destroy_on_destructor(false),
	vma_alloc(std::nullopt)
	{
		*this = std::move(move);
	}

	Image::~Image()
	{
		if(this->destroy_on_destructor && this->image != VK_NULL_HANDLE)
		{
			tz_assert(this->device != nullptr && !this->device->is_null(), "Tried to destroy VkImage but LogicalDevice is nullptr or null");
			tz_assert(this->vma_alloc.has_value(), "Image destructor thinks that the image does need to be destroyed, but there is no associated VmaAllocation. This means we cannot destroy the image. Please submit a bug report.");
			vmaDestroyImage(this->get_device().vma_native(), this->image, this->vma_alloc.value());
			this->image = VK_NULL_HANDLE;
		}
	}

	Image& Image::operator=(Image&& rhs)
	{
		std::swap(this->image, rhs.image);
		std::swap(this->format, rhs.format);
		std::swap(this->layout, rhs.layout);
		std::swap(this->device, rhs.device);
		std::swap(this->destroy_on_destructor, rhs.destroy_on_destructor);
		std::swap(this->vma_alloc, rhs.vma_alloc);
		return *this;
	}

	ImageFormat Image::get_format() const
	{
		return this->format;
	}

	ImageLayout Image::get_layout() const
	{
		return this->layout;
	}

	Vec2ui Image::get_dimensions() const
	{
		return this->dimensions;
	}

	const LogicalDevice& Image::get_device() const
	{
		tz_assert(this->device != nullptr, "Image had nullptr or null LogicalDevice");
		return *this->device;
	}

	Image::NativeType Image::native() const
	{
		return this->image;
	}

	Image Image::null()
	{
		return {};
	}

	bool Image::is_null() const
	{
		return this->image == VK_NULL_HANDLE;
	}

	bool Image::operator==(const Image& rhs) const
	{
		return this->image == rhs.image;
	}

	Image::Image():
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	dimensions{0u, 0u},
	device(nullptr),
	destroy_on_destructor(false)
	{}

	void Image::set_layout(ImageLayout layout)
	{
		this->layout = layout;
	}
}

#endif // TZ_VULKAN
