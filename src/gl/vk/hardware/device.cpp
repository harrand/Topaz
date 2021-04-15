#include "gl/vk/hardware/device.hpp"
#include "core/assert.hpp"
#if TZ_VULKAN

namespace tz::gl::vk::hardware
{
    Device Device::null()
    {
        return {};
    }

    bool Device::is_null() const
    {
        return this->dev == VK_NULL_HANDLE;
    }

    DeviceProperties Device::get_properties() const
    {
        tz_assert(!this->is_null(), "tz::gl::vk::hardware::Device::get_properties(): Null device handle.", "");
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(this->dev, &props);
        return props;
    }

    Device::Device():
    Device(VK_NULL_HANDLE)
    {

    }

    Device::Device(VkPhysicalDevice phys_dev):
    dev(phys_dev){}

    DeviceList get_all_devices(VkInstance instance)
    {
        DeviceList devices;

        std::uint32_t num_devices;
        vkEnumeratePhysicalDevices(instance, &num_devices, nullptr);
        tz_assert(num_devices > 0, "tz::gl::vk::hardware::get_all_devices(...): No physical devices were found. Rendering is impossible.", "");
        std::vector<VkPhysicalDevice> physical_devices(num_devices);
        vkEnumeratePhysicalDevices(instance, &num_devices, physical_devices.data());
        for(const auto& dev_handle : physical_devices)
        {
            Device cur_dev{dev_handle};
            devices.add(cur_dev);
        }
        return devices;
    }
}

#endif // TZ_VULKAN