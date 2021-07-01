#ifndef TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP
#define TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP
#if TZ_VULKAN
#include "vk_mem_alloc.h"
#include "gl/vk/hardware/device.hpp"
#include "gl/vk/setup/extension_list.hpp"
#include "gl/vk/hardware/queue.hpp"
#include "gl/vk/tz_vulkan.hpp"

namespace tz::gl::vk
{

    class LogicalDevice
    {
    public:
        LogicalDevice(hardware::DeviceQueueFamily queue_family, ExtensionList device_extensions = {}, VkPhysicalDeviceFeatures features = vk::required_rendering_features());
        LogicalDevice(const LogicalDevice& copy) = delete;
        LogicalDevice(LogicalDevice&& move);
        ~LogicalDevice();

        LogicalDevice& operator=(const LogicalDevice& rhs) = delete;
        LogicalDevice& operator=(LogicalDevice&& rhs);

        static LogicalDevice null();
        bool is_null() const;

        const hardware::DeviceQueueFamily& get_queue_family() const;
        VkDevice native() const;
        VmaAllocator native_allocator() const;
        hardware::Queue get_hardware_queue(std::uint32_t family_index = 0) const;

        void block_until_idle() const;
    private:
        LogicalDevice();

        VkDevice dev;
        hardware::DeviceQueueFamily queue_family;
        std::optional<VmaAllocator> vma;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP