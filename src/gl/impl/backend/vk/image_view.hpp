#ifndef TOPAZ_GL_VK_SETUP_IMAGE_VIEW_HPP
#define TOPAZ_GL_VK_SETUP_IMAGE_VIEW_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include "gl/impl/backend/vk/logical_device.hpp"
#include "gl/impl/backend/vk/image.hpp"

namespace tz::gl::vk
{
	class ImageView
	{
	public:
		ImageView(const LogicalDevice& device, const Image& image);
		ImageView(const ImageView& copy) = delete;
		ImageView(ImageView&& move);
		~ImageView();

		ImageView& operator=(const ImageView& rhs) = delete;
		ImageView& operator=(ImageView&& rhs);

		const Image* get_image() const;
		VkImageView native() const;
	private:        
		VkImageView view;
		const Image* image;
		const LogicalDevice* device;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_IMAGE_VIEW_HPP