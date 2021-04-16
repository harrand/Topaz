#ifndef TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP
#define TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP
#if TZ_VULKAN
#include "gl/vk/hardware/device.hpp"

namespace tz::gl::vk
{

    class LogicalDevice
    {
    public:
        LogicalDevice(hardware::DeviceQueueFamily queue_family);
        LogicalDevice(const LogicalDevice& copy) = delete;
        LogicalDevice(LogicalDevice&& move);
        ~LogicalDevice();

        LogicalDevice& operator=(const LogicalDevice& rhs) = delete;
        LogicalDevice& operator=(LogicalDevice&& rhs);
    private:
        VkDevice dev;
        VkQueue queue;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_SETUP_LOGICAL_DEVICE_HPP