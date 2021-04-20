#if TZ_VULKAN
#include "gl/vk/impl/setup/window_surface.hpp"
#include "core/tz.hpp"
#include "gl/vk/tz_vulkan.hpp"

namespace tz::gl::vk
{
    WindowSurface::WindowSurface():
    surface(VK_NULL_HANDLE)
    {
        glfwCreateWindowSurface(tz::gl::vk::get().native(), tz::window().get_middleware_handle(), nullptr, &this->surface);
    }

    WindowSurface::WindowSurface(WindowSurface&& move):
    surface(VK_NULL_HANDLE)
    {
        std::swap(this->surface, move.surface);
    }

    WindowSurface::~WindowSurface()
    {
        if(this->surface == VK_NULL_HANDLE)
        {
            return;
        }
        vkDestroySurfaceKHR(tz::gl::vk::get().native(), this->surface, nullptr);
        this->surface = VK_NULL_HANDLE;
    }

    WindowSurface& WindowSurface::operator=(WindowSurface&& rhs)
    {
        std::swap(this->surface, rhs.surface);
        return *this;
    }

    VkSurfaceKHR WindowSurface::native() const
    {
        return this->surface;
    }
}

#endif // TZ_VULKAN