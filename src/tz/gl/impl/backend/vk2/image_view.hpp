#ifndef TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_VIEW_HPP
#define TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_VIEW_HPP
#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/image.hpp"

namespace tz::gl::vk2
{
	struct ImageViewInfo
	{
		Image* image;
		ImageAspectFlag aspect;
	};

	/**
	 * @ingroup tz_gl_vk_image
	 */
	class ImageView : public DebugNameable<VK_OBJECT_TYPE_IMAGE_VIEW>
	{
	public:
		ImageView(ImageViewInfo info);
		ImageView(const ImageView& copy) = delete;
		ImageView(ImageView&& move);
		~ImageView();

		ImageView& operator=(const ImageView& rhs) = delete;
		ImageView& operator=(ImageView&& rhs);

		const Image& get_image() const;
		Image& get_image();

		ImageAspectFlag get_aspect() const;

		using NativeType = VkImageView;
		NativeType native() const;

		static ImageView null();
		bool is_null() const;
	private:
		ImageView();

		VkImageView image_view;
		ImageViewInfo info;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_IMPL_BACKEND_VK2_IMAGE_VIEW_HPP
