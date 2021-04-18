#if !TZ_VULKAN
    static_assert(false, "Cannot build vk_init_demo with TZ_VULKAN disabled.");
#endif

#include "core/tz.hpp"
#include "core/assert.hpp"
#include "gl/vk/setup/vulkan_instance.hpp"
#include "gl/vk/hardware/device.hpp"
#include "gl/vk/hardware/device_filter.hpp"
#include "gl/vk/setup/logical_device.hpp"

int main()
{
    constexpr tz::EngineInfo eng_info = tz::info();
    constexpr tz::GameInfo vk_window_demo{"vk_window_demo", eng_info.version, eng_info};
    tz::initialise(vk_window_demo);
    {
        using namespace tz::gl;
        vk::hardware::DeviceList valid_devices = tz::gl::vk::hardware::get_all_devices();
        vk::hardware::QueueFamilyTypeField type_requirements{{vk::hardware::QueueFamilyType::Graphics, vk::hardware::QueueFamilyType::Present}};
        // Let's grab a device which can do graphics and present images.
        // It also must support the swapchain device extension.
        {
            namespace hw = vk::hardware;
            hw::DeviceFilterList filters;
            filters.emplace<hw::DeviceQueueFamilyFilter>(type_requirements);
            filters.emplace<hw::DeviceExtensionSupportFilter>(std::initializer_list<vk::VulkanExtension>{"VK_KHR_swapchain"});
            filters.filter_all(valid_devices);
        }
        tz_assert(!valid_devices.empty(), "No valid devices. Require a physical device which supports graphics and present queue families.");
        // Just choose the first one.
        vk::hardware::Device my_device = valid_devices.front();
        std::optional<vk::hardware::DeviceQueueFamily> maybe_my_qfam = std::nullopt;
        for(auto fam : my_device.get_queue_families())
        {
            if(fam.types_supported.contains(type_requirements))
            {
                maybe_my_qfam = fam;
            }
        }
        tz_assert(maybe_my_qfam.has_value(), "Valid device found which supports present and graphics, but not a single queue that can do both.");
        vk::hardware::DeviceQueueFamily my_qfam = maybe_my_qfam.value();

        // Now create the device which can do both. We also ask it to use the swapchain extension.
        vk::LogicalDevice my_logical_device{my_qfam, {"VK_KHR_swapchain"}};
        // Let's also check the extent to which this physical device supports swapchains
        vk::hardware::SwapchainSupportDetails swapchain_support = my_device.get_window_swapchain_support();
        tz_assert(swapchain_support.supports_swapchain, "Very odd. The logical device was spawned using swapchain, but the physical device apparantly doesn't support it after all? There's 99\% a bug somewhere");
        while(!tz::window().is_close_requested())
        {
            tz::window().update();
        }
    }
    tz::terminate();
    return 0;
}