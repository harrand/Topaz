#if !TZ_VULKAN
    static_assert(false, "Cannot build vk_init_demo with TZ_VULKAN disabled.");
#endif

#include "core/tz.hpp"
#include "gl/vk/setup/vulkan_instance.hpp"

int main()
{
    constexpr tz::EngineInfo eng_info = tz::info();
    constexpr tz::GameInfo vk_window_demo{"vk_window_demo", eng_info.version, eng_info};
    tz::initialise(vk_window_demo);
    {
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
        }
    }
    tz::terminate();
    return 0;
}