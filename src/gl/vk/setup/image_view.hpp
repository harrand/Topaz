#ifndef TOPAZ_GL_VK_SETUP_IMAGE_VIEW_HPP
#define TOPAZ_GL_VK_SETUP_IMAGE_VIEW_HPP
#if TZ_VULKAN
#include "gl/vk/setup/logical_device.hpp"

namespace tz::gl::vk
{
    class ImageView
    {
    public:
        ImageView(const LogicalDevice& device, VkImage image, VkFormat format);
        ImageView(const ImageView& copy) = delete;
        ImageView(ImageView&& move);
        ~ImageView();

        ImageView& operator=(const ImageView& rhs) = delete;
        ImageView& operator=(ImageView&& rhs);
    private:
        VkImageView view;
        const LogicalDevice* device;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_IMAGE_VIEW_HPP