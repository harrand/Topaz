#ifndef TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP
#define TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP
#if TZ_VULKAN
#include "gl/vk/hardware/device.hpp"
#include "gl/vk/setup/extension_list.hpp"

namespace tz::gl::vk
{

    class LogicalDevice
    {
    public:
        LogicalDevice(hardware::DeviceQueueFamily queue_family, ExtensionList device_extensions = {});
        LogicalDevice(const LogicalDevice& copy) = delete;
        LogicalDevice(LogicalDevice&& move);
        ~LogicalDevice();

        LogicalDevice& operator=(const LogicalDevice& rhs) = delete;
        LogicalDevice& operator=(LogicalDevice&& rhs);

        const hardware::DeviceQueueFamily& get_queue_family() const;
        VkDevice native() const;
    private:
        VkDevice dev;
        VkQueue queue;
        hardware::DeviceQueueFamily queue_family;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP