#if TZ_VULKAN
#include "gl/impl/backend/vk2/image.hpp"
#include "gl/impl/backend/vk2/swapchain.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	Image::Image(SwapchainImageInfo sinfo):
	image(VK_NULL_HANDLE),
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
		this->device = &sinfo.swapchain->get_device();
	}

	Image::Image(Image&& move):
	image(VK_NULL_HANDLE),
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
		std::swap(this->device, rhs.device);
		std::swap(this->destroy_on_destructor, rhs.destroy_on_destructor);
		return *this;
	}
}

#endif // TZ_VULKAN
