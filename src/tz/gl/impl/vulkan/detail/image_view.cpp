#if TZ_VULKAN
#include "tz/core/profile.hpp"
#include "tz/gl/impl/vulkan/detail/image_view.hpp"
#include "tz/gl/impl/vulkan/detail/logical_device.hpp"

namespace tz::gl::vk2
{
	ImageView::ImageView(ImageViewInfo info):
	DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>(info.image->get_device()),
	image_view(VK_NULL_HANDLE),
	info(info),
	ldev(&this->info.image->get_device())
	{
		TZ_PROFZONE("Vulkan Backend - ImageView Create", 0xFFAA0000);
		tz::assert(this->info.image != nullptr && !this->info.image->is_null(), "ImageViewInfo refers to nullptr or null Image");
		VkImageViewCreateInfo create{};
		create.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; 
		create.image = this->info.image->native();
		create.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create.format = static_cast<VkFormat>(this->info.image->get_format());
		switch(info.aspect)
		{
			case ImageAspectFlag::Colour:
				create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
			case ImageAspectFlag::Depth:
				create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
			default:
				tz::error("Unknown ImageAspectFlag");
			break;
		}
		create.subresourceRange.baseMipLevel = 0;
		create.subresourceRange.levelCount = 1;
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
				tz::error("Ran out of host memory (RAM) while trying to create ImageView. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Ran out of device memory (VRAM) while trying to create ImageView. Ensure your system meets the minimum requirements.");
			break;
			default:
				tz::error("Failed to create ImageView, but the error code is unrecognised. Please submit a bug report.");
			break;
		}
		DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>::debug_set_handle(reinterpret_cast<std::uint64_t>(this->image_view));
		if(!this->get_image().debug_get_name().empty())
		{
			DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>::debug_set_name("View : " + this->get_image().debug_get_name());
		}
	}

	ImageView::ImageView(ImageView&& move):
	image_view(VK_NULL_HANDLE),
	info(),
	ldev(nullptr)
	{
		*this = std::move(move);
	}

	ImageView::~ImageView()
	{
		if(this->image_view != VK_NULL_HANDLE)
		{
			tz::assert(this->ldev != nullptr);
			vkDestroyImageView(this->ldev->native(), this->image_view, nullptr);
			this->image_view = VK_NULL_HANDLE;
		}
	}

	ImageView& ImageView::operator=(ImageView&& rhs)
	{
		std::swap(this->image_view, rhs.image_view);
		std::swap(this->info, rhs.info);
		std::swap(this->ldev, rhs.ldev);
		DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>::debugname_swap(rhs);
		return *this;
	}

	const Image& ImageView::get_image() const
	{
		tz::assert(this->info.image != nullptr, "ImageView originated from a nullptr Image. Move semantics may have gone pear-shaped. Please submit a bug report.");
		return *this->info.image;
	}

	Image& ImageView::get_image()
	{
		tz::assert(this->info.image != nullptr, "ImageView originated from a nullptr Image. Move semantics may have gone pear-shaped. Please submit a bug report.");
		return *this->info.image;
	}

	ImageAspectFlag ImageView::get_aspect() const
	{
		return this->info.aspect;
	}

	ImageView::NativeType ImageView::native() const
	{
		return this->image_view;
	}

	ImageView ImageView::null()
	{
		return {};
	}

	bool ImageView::is_null() const
	{
		return this->image_view == VK_NULL_HANDLE;
	}

	ImageView::ImageView():
	image_view(VK_NULL_HANDLE),
	info(){}
}

#endif // TZ_VULKAN
