#ifndef TOPAZ_GL_VK_HARDWARE_DEVICE_HPP
#define TOPAZ_GL_VK_HARDWARE_DEVICE_HPP
#if TZ_VULKAN
#include "gl/vk/common/basic_list.hpp"
#include "gl/vk/hardware/queue_family.hpp"

namespace tz::gl::vk::hardware
{
    using DeviceProperties = VkPhysicalDeviceProperties;
    using DeviceQueueFamilies = tz::gl::vk::common::BasicList<DeviceQueueFamily>;

    class Device
    {
    public:
        using List = tz::gl::vk::common::BasicList<Device>;
        static Device null();
        bool is_null() const;
        DeviceProperties get_properties() const;
        DeviceQueueFamilies get_queue_families() const;

        friend List get_all_devices();
    private:
        Device();
        Device(VkPhysicalDevice phys_dev);

        VkPhysicalDevice dev;
    };
    
    using DeviceList = Device::List;

    DeviceList get_all_devices();
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_HARDWARE_DEVICE_HPP