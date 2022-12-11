#if TZ_VULKAN
#include "hdk/profile.hpp"
#include "hdk/debug.hpp"
#include "tz/gl/impl/backend/vk2/image.hpp"
#include "tz/gl/impl/backend/vk2/swapchain.hpp"
#include "tz/gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	ImageAspectFlags derive_aspect_from_format(ImageFormat fmt)
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
	Image::Image(SwapchainImageInfo sinfo):
	DebugNameable<VK_OBJECT_TYPE_IMAGE>(sinfo.swapchain->get_device()),
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	tiling(ImageTiling::Optimal),
	residency(MemoryResidency::GPU),
	dimensions(),
	device(nullptr),
	destroy_on_destructor(false),
	vma_alloc(std::nullopt),
	vma_alloc_info()
	{
		HDK_PROFZONE("Vulkan Backend - Swapchain Image Create", 0xFFAA0000);
		hdk::assert(sinfo.swapchain != nullptr && !sinfo.swapchain->is_null(), "SwapchainImageInfo had nullptr or null Swapchain");
		std::uint32_t real_swapchain_image_count;
		std::vector<VkImage> swapchain_image_natives;

		vkGetSwapchainImagesKHR(sinfo.swapchain->get_device().native(), sinfo.swapchain->native(), &real_swapchain_image_count, nullptr);
		swapchain_image_natives.resize(real_swapchain_image_count);
		vkGetSwapchainImagesKHR(sinfo.swapchain->get_device().native(), sinfo.swapchain->native(), &real_swapchain_image_count, swapchain_image_natives.data());

		hdk::assert(std::cmp_less(sinfo.image_index, real_swapchain_image_count), "SwapchainImageInfo image index %u is out of range of the Swapchain's true number of images (%u).", sinfo.image_index, real_swapchain_image_count);
		
		// Now actually initialise the object internals.
		this->image = swapchain_image_natives[sinfo.image_index];
		this->format = sinfo.swapchain->get_image_format();
		this->layout = ImageLayout::Undefined;
		this->dimensions = sinfo.swapchain->get_dimensions();
		this->device = &sinfo.swapchain->get_device();
		// I verified in the spec here: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#_wsi_swapchain that the initial layout of a swapchain image is guaranteed to be undefined.
		DebugNameable<VK_OBJECT_TYPE_IMAGE>::debug_set_handle(reinterpret_cast<std::uint64_t>(this->image));
	}

	Image::Image(ImageInfo info):
	DebugNameable<VK_OBJECT_TYPE_IMAGE>(*info.device),
	image(VK_NULL_HANDLE),
	format(info.format),
	layout(ImageLayout::Undefined),
	tiling(info.image_tiling),
	residency(info.residency),
	dimensions(info.dimensions),
	device(info.device),
	destroy_on_destructor(true),
	vma_alloc(VmaAllocation{}),
	vma_alloc_info()
	{
		HDK_PROFZONE("Vulkan Backend - Image Create", 0xFFAA0000);
		VkImageCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = static_cast<VkFormat>(this->format),
			.extent = {.width = info.dimensions[0], .height = info.dimensions[1], .depth = 1},
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
			default:
				hdk::error("Unrecognised MemoryResidency. Please submit a bug report.");
			[[fallthrough]];
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

		VkResult res = vmaCreateImage(this->get_device().vma_native(), &create, &alloc, &this->image, &this->vma_alloc.value(), &this->vma_alloc_info);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				hdk::error("Failed to create Image because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Failed to create Image because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				hdk::error("Undocumented return value VK_ERROR_FEATURE_NOT_PRESENT from vmaCreateImage. TODO: Invoke vkGetPhysicalDeviceImageFormatProperties before trying to create the image to catch this early. The combination of format, tiling, residency etc are unsupported for this specific machine. Please submit a bug report.");
			break;
			default:
				hdk::error("Failed to create Image but cannot determine why. Please submit a bug report.");
			break;
		}
		DebugNameable<VK_OBJECT_TYPE_IMAGE>::debug_set_handle(reinterpret_cast<std::uint64_t>(this->image));
	}

	Image::Image(Image&& move):
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	tiling(),
	residency(),
	dimensions(0u, 0u),
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
			hdk::assert(this->device != nullptr && !this->device->is_null(), "Tried to destroy VkImage but LogicalDevice is nullptr or null");
			hdk::assert(this->vma_alloc.has_value(), "Image destructor thinks that the image does need to be destroyed, but there is no associated VmaAllocation. This means we cannot destroy the image. Please submit a bug report.");
			vmaDestroyImage(this->get_device().vma_native(), this->image, this->vma_alloc.value());
			this->image = VK_NULL_HANDLE;
		}
	}

	Image& Image::operator=(Image&& rhs)
	{
		std::swap(this->image, rhs.image);
		std::swap(this->format, rhs.format);
		std::swap(this->layout, rhs.layout);
		std::swap(this->tiling, rhs.tiling);
		std::swap(this->residency, rhs.residency);
		std::swap(this->dimensions, rhs.dimensions);
		std::swap(this->device, rhs.device);
		std::swap(this->destroy_on_destructor, rhs.destroy_on_destructor);
		std::swap(this->vma_alloc, rhs.vma_alloc);
		std::swap(this->vma_alloc_info, rhs.vma_alloc_info);
		DebugNameable<VK_OBJECT_TYPE_IMAGE>::debugname_swap(rhs);
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

	hdk::vec2ui Image::get_dimensions() const
	{
		return this->dimensions;
	}

	const LogicalDevice& Image::get_device() const
	{
		hdk::assert(this->device != nullptr, "Image had nullptr or null LogicalDevice");
		return *this->device;
	}

	void* Image::map()
	{
		HDK_PROFZONE("Vulkan Backend - Image Map", 0xFFAA0000);
		if(!this->vma_alloc.has_value() || this->residency == MemoryResidency::GPU)
		{
			return nullptr;
		}
		if(this->tiling != ImageTiling::Linear)
		{
			hdk::report("Mapping image memory, but its tiling is not linear. You cannot predict the internal data format so writes are likely to be invalid");
		}
		// If we've mapped earlier/are CPUPersistent, we already have a ptr we can use.
		if(this->vma_alloc_info.pMappedData != nullptr)
		{
			return this->vma_alloc_info.pMappedData;
		}
		// Otherwise we'll actually perform the map
		VkResult res = vmaMapMemory(this->get_device().vma_native(), this->vma_alloc.value(), &this->vma_alloc_info.pMappedData);
		if(res == VK_SUCCESS)
		{
			return this->vma_alloc_info.pMappedData;
		}
		return nullptr;
	}

	void Image::unmap()
	{
		HDK_PROFZONE("Vulkan Backend - Image Unmap", 0xFFAA0000);
		if(!this->vma_alloc.has_value() || this->vma_alloc_info.pMappedData == nullptr || this->residency == MemoryResidency::CPUPersistent)
		{
			return;
		}

		vmaUnmapMemory(this->get_device().vma_native(), this->vma_alloc.value());
		this->vma_alloc_info.pMappedData = nullptr;
	}

	std::size_t Image::get_linear_row_length() const
	{
		VkSubresourceLayout data_layout;
		VkImageSubresource subresource
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.arrayLayer = 0
		};
		vkGetImageSubresourceLayout(this->get_device().native(), this->native(), &subresource, &data_layout);
		// row pitch is the number of bytes between each row.
		return data_layout.rowPitch;
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
