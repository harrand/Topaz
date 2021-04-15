#if !TZ_VULKAN
    static_assert(false, "Cannot build vk_init_demo with TZ_VULKAN disabled.");
#endif

#include "gl/vk/setup/vulkan_instance.hpp"

int main()
{
    constexpr tz::EngineInfo eng_info = tz::info();
    tz::GameInfo vk_init_demo{"vk_init_demo", eng_info.version, eng_info};
    namespace tzvk = tz::gl::vk;
    tzvk::VulkanApplicationInfo vk_info{vk_init_demo};

    tzvk::VulkanInstance vk_inst{vk_info};
    return 0;
}