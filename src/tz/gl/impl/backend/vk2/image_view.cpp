#if TZ_VULKAN
#include "hdk/profile.hpp"
#include "tz/gl/impl/backend/vk2/image_view.hpp"
#include "tz/gl/impl/backend/vk2/logical_device.hpp"

namespace tz::gl::vk2
{
	ImageView::ImageView(ImageViewInfo info):
	DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>(info.image->get_device()),
	image_view(VK_NULL_HANDLE),
	info(info)
	{
		HDK_PROFZONE("Vulkan Backend - ImageView Create", 0xFFAA0000);
		hdk::assert(this->info.image != nullptr && !this->info.image->is_null(), "ImageViewInfo refers to nullptr or null Image");
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
				hdk::error("Unknown ImageAspectFlag");
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
				hdk::error("Ran out of host memory (RAM) while trying to create ImageView. Ensure your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				hdk::error("Ran out of device memory (VRAM) while trying to create ImageView. Ensure your system meets the minimum requirements.");
			break;
			default:
				hdk::error("Failed to create ImageView, but the error code is unrecognised. Please submit a bug report.");
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

	ImageView& ImageView::operator=(ImageView&& rhs)
	{
		std::swap(this->image_view, rhs.image_view);
		std::swap(this->info, rhs.info);
		DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>::debugname_swap(rhs);
		return *this;
	}

	const Image& ImageView::get_image() const
	{
		hdk::assert(this->info.image != nullptr, "ImageView originated from a nullptr Image. Move semantics may have gone pear-shaped. Please submit a bug report.");
		return *this->info.image;
	}

	Image& ImageView::get_image()
	{
		hdk::assert(this->info.image != nullptr, "ImageView originated from a nullptr Image. Move semantics may have gone pear-shaped. Please submit a bug report.");
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
