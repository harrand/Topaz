#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    LogicalDevice::LogicalDevice(hardware::DeviceQueueFamily queue_family, ExtensionList device_extensions):
    dev(VK_NULL_HANDLE),
    queue_family(queue_family)
    {
        VkDeviceQueueCreateInfo queue_create{};
        queue_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create.queueFamilyIndex = queue_family.index;
        queue_create.queueCount = 1;

        float queue_priority = 1.0f;
        queue_create.pQueuePriorities = &queue_priority;

        VkPhysicalDeviceFeatures features{};
        features.samplerAnisotropy = VK_FALSE; // TODO: Customiseable?

        VkDeviceCreateInfo create{};
        create.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create.pQueueCreateInfos = &queue_create;
        create.queueCreateInfoCount = 1;
        create.pEnabledFeatures = &features;

        create.enabledExtensionCount = device_extensions.length();
        create.ppEnabledExtensionNames = device_extensions.data();
        // Note: In Vulkan 1.1.175, device-only layers are not a thing, they just use the instance layers.
        create.enabledLayerCount = 0;

        VkResult res = vkCreateDevice(queue_family.dev->native(), &create, nullptr, &this->dev);
        tz_assert(res == VK_SUCCESS, "tz::gl::vk::LogicalDevice(...): Failed to create logical device.");
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& move):
    dev(VK_NULL_HANDLE),
    queue_family()
    {
        *this = std::move(move);
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
        std::swap(this->queue_family, rhs.queue_family);
        return *this;
    }

    const hardware::DeviceQueueFamily& LogicalDevice::get_queue_family() const
    {
        return this->queue_family;
    }

    VkDevice LogicalDevice::native() const
    {
        return this->dev;
    }

    hardware::Queue LogicalDevice::get_hardware_queue(std::uint32_t family_index) const
    {
        return {*this, this->get_queue_family(), family_index};
    }

    void LogicalDevice::block_until_idle() const
    {
        vkDeviceWaitIdle(this->dev);
    }
}

#endif