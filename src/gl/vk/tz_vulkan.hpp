#ifndef TOPAZ_GL_VK_TZ_VULKAN_HPP
#define TOPAZ_GL_VK_TZ_VULKAN_HPP
#if TZ_VULKAN
#include "gl/vk/setup/vulkan_instance.hpp"

namespace tz::gl::vk
{
    void initialise(tz::GameInfo game_info);
    void terminate();
    VulkanInstance& get();
    bool is_initialised();
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_TZ_VULKAN_HPP