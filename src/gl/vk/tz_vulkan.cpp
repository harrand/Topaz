#if TZ_VULKAN
#include "gl/vk/tz_vulkan.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    VulkanInstance* inst = nullptr;
    void initialise(tz::GameInfo game_info)
    {
        tz_assert(inst == nullptr, "tz::gl::vk::initialise(...): Already initialised", "");
        inst = new VulkanInstance{game_info};
    }

    void terminate()
    {
        tz_assert(inst != nullptr, "tz::gl::vk::terminate(): Not initialised", "");
        delete inst;
        inst = nullptr;
    }

    VulkanInstance& get()
    {
        tz_assert(inst != nullptr, "tz::gl::vk::get(): Never initialised", "");
        return *inst;
    }

    bool is_initialised()
    {
        return inst != nullptr;
    }
}

#endif // TZ_VULKAN