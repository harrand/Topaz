#if TZ_VULKAN
#include "gl/vk/tz_vulkan.hpp"
#include "core/assert.hpp"
#include "core/report.hpp"
#include <cstdio>

namespace tz::gl::vk
{
    VulkanInstance* inst = nullptr;
    WindowSurface* surf = nullptr;

    void initialise_headless(tz::GameInfo game_info)
    {
        tz_assert(inst == nullptr, "tz::gl::vk::initialise(...): Already initialised");
        inst = new VulkanInstance{game_info};
        tz_report("Vulkan Initialised (Headless)");
    }

    void initialise(tz::GameInfo game_info)
    {
        tz_assert(inst == nullptr, "tz::gl::vk::initialise(...): Already initialised");
        inst = new VulkanInstance{game_info};
        surf = new WindowSurface;
        tz_report("Vulkan Initialised (Window)");
    }

    void terminate()
    {
        tz_assert(inst != nullptr, "tz::gl::vk::terminate(): Not initialised");
        delete surf;
        surf = nullptr;
        delete inst;
        inst = nullptr;
        tz_report("Vulkan Terminated");
    }

    VulkanInstance& get()
    {
        tz_assert(inst != nullptr, "tz::gl::vk::get(): Never initialised");
        return *inst;
    }

    bool is_initialised()
    {
        return inst != nullptr;
    }

    WindowSurface* window_surface()
    {
        return surf;
    }

    bool is_headless()
    {
        return surf == nullptr;
    }
}

#endif // TZ_VULKAN