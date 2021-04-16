#if TZ_VULKAN
#include "gl/vk/setup/logical_device.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    LogicalDevice::LogicalDevice(hardware::DeviceQueueFamily queue_family)
    {
        // TODO: Remove assert?
        tz_assert(queue_family.types_supported.contains(hardware::QueueFamilyType::Graphics), "tz::gl::vk::LogicalDevice::LogicalDevice(...): The given queue family must support graphics queues. Although I admit this might not be a reasonable assert?", "");
        VkDeviceQueueCreateInfo queue_create{};
        queue_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create.queueFamilyIndex = queue_family.index;
        queue_create.queueCount = 1;

        float queue_priority = 1.0f;
        queue_create.pQueuePriorities = &queue_priority;

        // TODO: Populate this once we start properly using Vulkan.
        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create.pQueueCreateInfos = &queue_create;
        create.queueCreateInfoCount = 1;
        create.pEnabledFeatures = &features;

        // TODO: Device Specific Extension (such as VK_KHR_swapchain which we will need further down development)
        create.enabledExtensionCount = 0;
        // Note: In Vulkan 1.1.175, device-only layers are not a thing, they just use the instance layers.
        // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
        // For that reason, we assume no version is used before this
        // TODO: Require this in CMake.
        create.enabledLayerCount = 0;

        VkResult res = vkCreateDevice(queue_family.dev, &create, nullptr, &this->dev);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::LogicalDevice(...): Failed to create logical device.", "");

        // Possible TODO: Support multiple queues?
        vkGetDeviceQueue(this->dev, queue_family.index, 0, &this->queue);
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& move):
    dev(VK_NULL_HANDLE)
    {
        std::swap(this->dev, move.dev);
    }

    LogicalDevice::~LogicalDevice()
    {
        if(this->dev != VK_NULL_HANDLE)
        {
            vkDestroyDevice(this->dev, nullptr);
        }
    }

    LogicalDevice& LogicalDevice::operator=(LogicalDevice&& rhs)
    {
        std::swap(this->dev, rhs.dev);
    }
}

#endif