#if TZ_VULKAN
#include "gl/vk/hardware/device_filter.hpp"

namespace tz::gl::vk::hardware
{
    bool DeviceFilterList::satisfies(const hardware::Device& device) const
    {
        for(auto iter = this->begin(); iter != this->end(); iter++)
        {
            if(!iter->satisfies(device))
            {
                return false;
            }
        }
        return true;
    }

    void DeviceFilterList::filter_all(hardware::DeviceList& devices) const
    {
        devices.erase(std::remove_if(devices.begin(), devices.end(), [this](const hardware::Device& dev){return !this->satisfies(dev);}), devices.end());
    }

    DeviceQueueFamilyFilter::DeviceQueueFamilyFilter(QueueFamilyTypeField types):
    types(types){}

    bool DeviceQueueFamilyFilter::satisfies(const hardware::Device& device) const
    {
        auto families = device.get_queue_families();
        for(DeviceQueueFamily family : families)
        {
            if(family.types_supported.contains(this->types))
            {
                return true;
            }
        }
        return false;
    }

    DeviceExtensionSupportFilter::DeviceExtensionSupportFilter(ExtensionList extension_names):
    required_extensions(extension_names){}

    bool DeviceExtensionSupportFilter::satisfies(const hardware::Device& device) const
    {
        auto extensions = device.get_extension_properties();
        for(VulkanExtension required_extension : this->required_extensions)
        {
            bool found = false;
            for(DeviceExtensionProperty supported_extension : extensions)
            {
                if(std::string{required_extension} == supported_extension.extensionName)
                {
                    found = true;
                }
            }
            if(!found)
            {
                return false;
            }
        }
        return true;
    }

    bool DeviceAnyGPUFilter::satisfies(const hardware::Device& device) const
    {
        return DeviceIntegratedGPUFilter{}.satisfies(device)
            || DeviceDiscreteGPUFilter{}.satisfies(device)
            || DeviceVirtualGPUFilter{}.satisfies(device);
    }

    namespace quick_filters
    {
        void remove_non_gpus(hardware::DeviceList& list)
        {
            DeviceFilterList filters;
            filters.emplace<DeviceAnyGPUFilter>();
            filters.filter_all(list);
        }

        void preserve_only_discrete_gpus(hardware::DeviceList& list)
        {
            DeviceFilterList filters;
            filters.emplace<DeviceDiscreteGPUFilter>();
            filters.filter_all(list);
        }

        void preserve_only_integrated_gpus(hardware::DeviceList& list)
        {
            DeviceFilterList filters;
            filters.emplace<DeviceIntegratedGPUFilter>();
            filters.filter_all(list);
        }

        void preserve_only_with_appropriate_queue(hardware::DeviceList& list, QueueFamilyTypeField field)
        {
            DeviceFilterList filters;
            filters.emplace<DeviceQueueFamilyFilter>(field);
            filters.filter_all(list);
        }

    }
}

#endif // TZ_VULKAN