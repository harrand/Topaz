#include "gl/impl/backend/vk/hardware/device.hpp"
#include "gl/impl/backend/vk/tz_vulkan.hpp"
#include "gl/impl/backend/vk/setup/window_surface.hpp"
#include "gl/impl/backend/vk/hardware/device_filter.hpp"
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
        tz_assert(!this->is_null(), "tz::gl::vk::hardware::Device::get_properties(): Null device handle.");
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
            DeviceQueueFamily fam{.dev = this, .index = i, .types_supported = {}};
            VkQueueFlags flag = prop.queueFlags;
            
            // The following queue family types are represented with a special bit.
            for(QueueFamilyType type : {QueueFamilyType::Graphics, QueueFamilyType::Compute, QueueFamilyType::Transfer, QueueFamilyType::SparseBinding})
            {
                VkQueueFlags type_as_flag;
                switch(type)
                {
                    case QueueFamilyType::Graphics:
                        type_as_flag = VK_QUEUE_GRAPHICS_BIT;
                    break;
                    case QueueFamilyType::Compute:
                        type_as_flag = VK_QUEUE_COMPUTE_BIT;
                    break;
                    case QueueFamilyType::Transfer:
                        type_as_flag = VK_QUEUE_TRANSFER_BIT;
                    break;
                    case QueueFamilyType::SparseBinding:
                        type_as_flag = VK_QUEUE_SPARSE_BINDING_BIT;
                    break;
                    case QueueFamilyType::Present:
                        tz_error("Did not expect Present here");
                    break;
                }
                if(flag & type_as_flag)
                {
                    fam.types_supported |= type;
                }
            }
            // The other queue family types are defined separately.
            // Present Flag (device supports window system integration) -- Note that this is disabled if headless rendering is enabled.
            if(!tz::gl::vk::is_headless())
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(this->dev, i, tz::gl::vk::window_surface()->native(), &present_support);
                if(present_support)
                {
                    fam.types_supported |= QueueFamilyType::Present;
                }
            }
            
            queue_families.add(fam);
            i++;
        }
        return queue_families;
    }

    DeviceExtensionProperties Device::get_extension_properties() const
    {
        std::uint32_t supported_extension_count;
        vkEnumerateDeviceExtensionProperties(this->dev, nullptr, &supported_extension_count, nullptr);
        DeviceExtensionProperties supported_extensions;
        supported_extensions.resize(supported_extension_count);
        vkEnumerateDeviceExtensionProperties(this->dev, nullptr, &supported_extension_count, supported_extensions.data());
        return supported_extensions;
    }

    SwapchainSupportDetails Device::get_window_swapchain_support() const
    {
        return
        {
            this->get_window_surface_capabilities(),
            this->get_window_surface_formats(),
            this->get_window_surface_present_modes(),
            this->supports_swapchain()
        };
    }

    MemoryProperties Device::get_memory_properties() const
    {
        return
        {
            *this
        };
    }


    VkPhysicalDevice Device::native() const
    {
        return this->dev;
    }

    Device::Device():
    Device(VK_NULL_HANDLE)
    {

    }

    Device::Device(VkPhysicalDevice phys_dev):
    dev(phys_dev){}

    bool Device::supports_swapchain() const
    {
        DeviceFilterList filter;
        filter.emplace<DeviceExtensionSupportFilter>(std::initializer_list<VulkanExtension>{"VK_KHR_swapchain"});
        return filter.satisfies(*this);
    }

    SurfaceCapabilities Device::get_surface_capabilities(VkSurfaceKHR custom_surface) const
    {
        tz_assert(this->supports_swapchain(), "Attempting to retrieve surface capabilities for this device, but there is no swapchain support");
        SurfaceCapabilities capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->dev, custom_surface, &capabilities);
        return capabilities;
    }

    SurfaceCapabilities Device::get_window_surface_capabilities() const
    {
        return this->get_surface_capabilities(vk::window_surface()->native());
    }

    SurfaceFormats Device::get_surface_formats(VkSurfaceKHR custom_surface) const
    {
        tz_assert(this->supports_swapchain(), "Attempting to retrieve surface formats for this device, but there is no swapchain support");
        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->dev, custom_surface, &format_count, nullptr);
        SurfaceFormats formats;
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->dev, custom_surface, &format_count, formats.data());
        return formats;
    }

    SurfaceFormats Device::get_window_surface_formats() const
    {
        return this->get_surface_formats(vk::window_surface()->native());
    }

    SurfacePresentModes Device::get_surface_present_modes(VkSurfaceKHR custom_surface) const
    {
        tz_assert(this->supports_swapchain(), "Attempting to retrieve surface present modes for this device, but there is no swapchain support");
        std::uint32_t present_modes_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->dev, custom_surface, &present_modes_count, nullptr);
        SurfacePresentModes present_modes;
        present_modes.resize(present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->dev, custom_surface, &present_modes_count, present_modes.data());
        return present_modes;
    }

    SurfacePresentModes Device::get_window_surface_present_modes() const
    {
        return this->get_surface_present_modes(vk::window_surface()->native());
    }

    DeviceList get_all_devices()
    {
        tz_assert(tz::gl::vk::is_initialised(), "tz::gl::vk::hardware::get_all_devices(): tz::gl::vk not initialised");
        DeviceList devices;

        std::uint32_t num_devices;
        vkEnumeratePhysicalDevices(tz::gl::vk::get().native(), &num_devices, nullptr);
        tz_assert(num_devices > 0, "tz::gl::vk::hardware::get_all_devices(...): No physical devices were found. Rendering is impossible.");
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