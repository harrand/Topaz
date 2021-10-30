#if TZ_VULKAN
#include "gl/impl/backend/vk2/image_view.hpp"
#include "gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	ImageView::ImageView(ImageViewInfo info):
	image_view(VK_NULL_HANDLE),
	info(info)
	{
		tz_assert(this->info.image != nullptr && !this->info.image->is_null(), "ImageViewInfo refers to nullptr or null Image");
		VkImageViewCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; 
		create.image = this->info.image->native();
		create.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create.format = static_cast<VkFormat>(this->info.image->get_format());
		switch(info.aspect)
		{
			case ImageAspect::Colour:
				create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
			case ImageAspect::Depth:
				create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
			default:
				tz_error("Unknown ImageAspect");
			break;
		}
		create.subresourceRange.baseMipLevel = 0;
		create.subresourceRange.levelCount = 0;
		create.subresourceRange.baseArrayLayer = 0;
		create.subresourceRange.layerCount = 1;

		const LogicalDevice& ldev = this->info.image->get_device();
		VkResult res = vkCreateImageView(ldev.native(), &create, nullptr, &this->image_view);
		switch(res)
		{
			case VK_SUCCESS:
				// do nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Ran out of host memory (RAM) while trying to create ImageView. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Ran out of device memory (VRAM) while trying to create ImageView. Ensure your system meets the minimum requirements.");
			break;
			default:
				tz_error("Failed to create ImageView, but the error code is unrecognised. Please submit a bug report.");
			break;
		}
	}

	ImageView::ImageView(ImageView&& move):
	image_view(VK_NULL_HANDLE),
	info()
	{
		*this = std::move(move);
	}

	ImageView::~ImageView()
	{
		if(this->image_view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(this->get_image().get_device().native(), this->image_view, nullptr);
			this->image_view = VK_NULL_HANDLE;
		}
	}

	const Image& ImageView::get_image() const
	{
		tz_assert(this->info.image != nullptr, "ImageView originated from a nullptr Image. Move semantics may have gone pear-shaped. Please submit a bug report.");
		return *this->info.image;
	}

	VkImageView ImageView::native() const
	{
		return this->image_view;
	}
}

#endif // TZ_VULKAN
