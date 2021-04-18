#ifndef TOPAZ_GL_VK_HARDWARE_DEVICE_HPP
#define TOPAZ_GL_VK_HARDWARE_DEVICE_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include "gl/vk/hardware/queue_family.hpp"
#include "gl/vk/hardware/swapchain_support.hpp"

namespace tz::gl::vk::hardware
{
    using DeviceProperties = VkPhysicalDeviceProperties;
    using DeviceExtensionProperty = VkExtensionProperties;
    using DeviceQueueFamilies = tz::BasicList<DeviceQueueFamily>;
    using DeviceExtensionProperties = tz::BasicList<DeviceExtensionProperty>;

    class Device
    {
    public:
        using List = tz::BasicList<Device>;
        static Device null();
        bool is_null() const;
        DeviceProperties get_properties() const;
        DeviceQueueFamilies get_queue_families() const;
        DeviceExtensionProperties get_extension_properties() const;
        SwapchainSupportDetails get_window_swapchain_support() const;

        friend List get_all_devices();
    private:
        Device();
        Device(VkPhysicalDevice phys_dev);

        bool supports_swapchain() const;
        SurfaceCapabilities get_surface_capabilities(VkSurfaceKHR custom_surface) const;
        SurfaceCapabilities get_window_surface_capabilities() const;
        SurfaceFormats get_surface_formats(VkSurfaceKHR custom_surface) const;
        SurfaceFormats get_window_surface_formats() const;
        SurfacePresentModes get_surface_present_modes(VkSurfaceKHR custom_surface) const;
        SurfacePresentModes get_window_surface_present_modes() const;


        VkPhysicalDevice dev;
    };
    
    using DeviceList = Device::List;

    DeviceList get_all_devices();
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_HARDWARE_DEVICE_HPP