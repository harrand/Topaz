#ifndef TOPAZ_GL_VK_TZ_VULKAN_HPP
#define TOPAZ_GL_VK_TZ_VULKAN_HPP
#if TZ_VULKAN
#include "gl/vk/impl/setup/vulkan_instance.hpp"
#include "gl/vk/impl/setup/window_surface.hpp"

namespace tz::gl::vk
{
    void initialise_headless(tz::GameInfo game_info);
    void initialise(tz::GameInfo game_info);
    void terminate();
    VulkanInstance& get();
    bool is_initialised();
    WindowSurface* window_surface();
    bool is_headless();
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_TZ_VULKAN_HPP