#include "gl/vk/hardware/device.hpp"
#include "gl/vk/tz_vulkan.hpp"
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

    DeviceQueueFamilies Device::get_queue_families() const
    {
        DeviceQueueFamilies queue_families;

        std::uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(this->dev, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_fams{queue_family_count};
        vkGetPhysicalDeviceQueueFamilyProperties(this->dev, &queue_family_count, queue_fams.data());
        int i = 0;
        for(const VkQueueFamilyProperties& prop : queue_fams)
        {
            DeviceQueueFamily fam{.dev = this->dev, .index = i};
            VkQueueFlags flag = prop.queueFlags;
            auto enum_2_flag = [](QueueFamilyType t){return static_cast<int>(t);};
            
            for(QueueFamilyType type : hardware::family_types)
            {
                if(flag & enum_2_flag(type))
                {
                    fam.types_supported |= type;
                }
            }
            queue_families.add(fam);
            i++;
        }
        return queue_families;
    }


    Device::Device():
    Device(VK_NULL_HANDLE)
    {

    }

    Device::Device(VkPhysicalDevice phys_dev):
    dev(phys_dev){}

    DeviceList get_all_devices()
    {
        tz_assert(tz::gl::vk::is_initialised(), "tz::gl::vk::hardware::get_all_devices(): tz::gl::vk not initialised", "");
        DeviceList devices;

        std::uint32_t num_devices;
        vkEnumeratePhysicalDevices(tz::gl::vk::get().native(), &num_devices, nullptr);
        tz_assert(num_devices > 0, "tz::gl::vk::hardware::get_all_devices(...): No physical devices were found. Rendering is impossible.", "");
        std::vector<VkPhysicalDevice> physical_devices(num_devices);
        vkEnumeratePhysicalDevices(tz::gl::vk::get().native(), &num_devices, physical_devices.data());
        for(const auto& dev_handle : physical_devices)
        {
            Device cur_dev{dev_handle};
            devices.add(cur_dev);
        }
        return devices;
    }
}

#endif // TZ_VULKAN