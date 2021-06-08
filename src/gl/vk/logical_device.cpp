#if TZ_VULKAN
#include "gl/vk/logical_device.hpp"
#include "gl/vk/tz_vulkan.hpp"
#include "core/assert.hpp"

namespace tz::gl::vk
{
    LogicalDevice::LogicalDevice(hardware::DeviceQueueFamily queue_family, ExtensionList device_extensions):
    dev(VK_NULL_HANDLE),
    queue_family(queue_family),
    vma(std::nullopt)
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

        VmaAllocatorCreateInfo alloc_create{};
        auto ToVkVersion = [](tz::Version ver)->std::uint32_t
        {
            return VK_MAKE_VERSION(ver.major, ver.minor, ver.patch);
        };
        alloc_create.vulkanApiVersion = ToVkVersion(vk::get_vulkan_version());
        alloc_create.physicalDevice = this->queue_family.dev->native();
        alloc_create.device = this->dev;
        alloc_create.instance = vk::get().native();

        this->vma = VmaAllocator{};
        res = vmaCreateAllocator(&alloc_create, &this->vma.value());
        tz_assert(res == VK_SUCCESS, "Failed to create vma allocator");
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& move):
    dev(VK_NULL_HANDLE),
    queue_family(),
    vma(std::nullopt)
    {
        *this = std::move(move);
    }

    LogicalDevice::~LogicalDevice()
    {
        if(this->vma.has_value())
        {
            vmaDestroyAllocator(this->vma.value());
            this->vma = std::nullopt;
        }
        if(this->dev != VK_NULL_HANDLE)
        {
            vkDestroyDevice(this->dev, nullptr);
        }
    }

    LogicalDevice& LogicalDevice::operator=(LogicalDevice&& rhs)
    {
        std::swap(this->dev, rhs.dev);
        std::swap(this->queue_family, rhs.queue_family);
        std::swap(this->vma, rhs.vma);
        return *this;
    }

    LogicalDevice LogicalDevice::null()
    {
        return {};
    }

    bool LogicalDevice::is_null() const
    {
        return this->dev == VK_NULL_HANDLE;
    }

    const hardware::DeviceQueueFamily& LogicalDevice::get_queue_family() const
    {
        return this->queue_family;
    }

    VkDevice LogicalDevice::native() const
    {
        return this->dev;
    }

    VmaAllocator LogicalDevice::native_allocator() const
    {
        tz_assert(this->vma.has_value(), "Native allocator doesn't exist.");
        return this->vma.value();
    }

    hardware::Queue LogicalDevice::get_hardware_queue(std::uint32_t family_index) const
    {
        return {*this, this->get_queue_family(), family_index};
    }

    void LogicalDevice::block_until_idle() const
    {
        vkDeviceWaitIdle(this->dev);
    }

    LogicalDevice::LogicalDevice():
    dev(VK_NULL_HANDLE),
    queue_family()
    {}
}

#endif