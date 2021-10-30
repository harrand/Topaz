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
	destroy_on_destructor(false)
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

	Image::Image(Image&& move):
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	device(nullptr),
	destroy_on_destructor(false)
	{
		*this = std::move(move);
	}

	Image::~Image()
	{
		if(this->destroy_on_destructor && this->image != VK_NULL_HANDLE)
		{
			tz_assert(this->device != nullptr && !this->device->is_null(), "Tried to destroy VkImage but LogicalDevice is nullptr or null");
			vkDestroyImage(this->device->native(), this->image, nullptr);
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

	VkImage Image::native() const
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

	Image::Image():
	image(VK_NULL_HANDLE),
	format(ImageFormat::Undefined),
	layout(ImageLayout::Undefined),
	dimensions{0u, 0u},
	device(nullptr),
	destroy_on_destructor(false)
	{}
}

#endif // TZ_VULKAN
