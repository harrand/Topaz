#ifndef TOPAZ_GL_VK_SETUP_WINDOW_SURFACE_HPP
#define TOPAZ_GL_VK_SETUP_WINDOW_SURFACE_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"

namespace tz::gl::vk
{
	class WindowSurface
	{
	public:
		WindowSurface();
		WindowSurface(const WindowSurface& copy) = delete;
		WindowSurface(WindowSurface&& move);
		~WindowSurface();
		WindowSurface& operator=(const WindowSurface& copy) = delete;
		WindowSurface& operator=(WindowSurface&& move);

		VkSurfaceKHR native() const;
	private:
		VkSurfaceKHR surface;
	};
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_WINDOW_SURFACE_HPP