#include "gl/device.hpp"

#if TZ_VULKAN
#include "gl/vk/hardware/device_filter.hpp"
#elif TZ_OGL

#endif

namespace tz::gl
{
#if TZ_VULKAN

    DeviceVulkan::DeviceVulkan():
    physical_device(vk::hardware::Device::null()),
    device(vk::LogicalDevice::null())
    {
        vk::hardware::DeviceList all_devices = vk::hardware::get_all_devices();
        // Ideally we want a queue which supports all 3 of graphics, transfer and present.
        vk::hardware::QueueFamilyTypeField type_requirements{{vk::hardware::QueueFamilyType::Graphics, vk::hardware::QueueFamilyType::Present, vk::hardware::QueueFamilyType::Transfer}};
        // We require swapchain support too.
        vk::hardware::DeviceFilterList filters;
        filters.emplace<vk::hardware::DeviceQueueFamilyFilter>(type_requirements);
        filters.emplace<vk::hardware::DeviceExtensionSupportFilter>(std::initializer_list<vk::VulkanExtension>{"VK_KHR_swapchain"});
        // Filter out all devices which don't match our description.
        filters.filter_all(all_devices);
        tz_assert(!all_devices.empty(), "Physical Device requirements are too stringent for this machine. Topaz Vulkan does not support your hardware.");
        // TODO: Rate all suitable physical devices and choose the best one.
        this->physical_device = all_devices.front();
        // Choose the queue family which is most suitable to our needs.
        std::optional<vk::hardware::DeviceQueueFamily> maybe_chosen_queue_family;
        for(vk::hardware::DeviceQueueFamily queue_family : this->physical_device.get_queue_families())
        {
            if(queue_family.types_supported.contains(type_requirements))
            {
                maybe_chosen_queue_family = queue_family;
            }
        }
        tz_assert(maybe_chosen_queue_family.has_value(), "Valid device found which supports present, graphics and transfer, but not a single queue that can do both. Topaz Vulkan does not support your hardware.");
        this->device = {maybe_chosen_queue_family.value(), {"VK_KHR_swapchain"}};
    }

    RenderPass DeviceVulkan::create_render_pass(RenderPassBuilder builder) const
    {
        builder.finalise();
        return {this->device, builder};
    }

#elif TZ_OGL

#endif
}