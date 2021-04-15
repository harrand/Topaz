#include "gl/vk/hardware/device_filter.hpp"
#if TZ_VULKAN

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
    }
}

#endif // TZ_VULKAN