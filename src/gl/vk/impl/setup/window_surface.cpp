#if TZ_VULKAN
#include "gl/vk/impl/setup/window_surface.hpp"
#include "core/tz.hpp"
#include "gl/vk/tz_vulkan.hpp"

namespace tz::gl::vk
{
    WindowSurface::WindowSurface():
    surface(VK_NULL_HANDLE)
    {
        auto res = glfwCreateWindowSurface(tz::gl::vk::get().native(), tz::window().get_middleware_handle(), nullptr, &this->surface);
        tz_assert(res == VK_SUCCESS, "GLFW Failed to create window surface");
    }

    WindowSurface::WindowSurface(WindowSurface&& move):
    surface(VK_NULL_HANDLE)
    {
        *this = std::move(move);
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