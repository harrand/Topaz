#include "gl/vk/hardware/queue_family.hpp"
#include <array>

namespace tz::gl::vk::hardware
{
    template<VkPhysicalDeviceType Type>
    bool DeviceTypeFilter<Type>::satisfies(const hardware::Device& device) const
    {
        return device.get_properties().deviceType == Type;
    }

    template<VkQueueFlagBits... QueueFamilyTypes>
    bool DeviceQueueFamilyFilter<QueueFamilyTypes...>::satisfies(const hardware::Device& device) const
    {
        constexpr std::array<VkQueueFlagBits, sizeof...(QueueFamilyTypes)> flags = {{QueueFamilyTypes...}};
        return hardware::supports_queue_flags(std::span(device.get_queue_families().data(), device.get_queue_families().length()), std::span(flags.data(), flags.size()));
    }

    namespace quick_filters
    {
        template<VkQueueFlagBits... QueueFamilyTypes>
        void preserve_only_supporting_queues(hardware::DeviceList& list)
        {
            DeviceFilterList filters;
            filters.emplace<DeviceQueueFamilyFilter<QueueFamilyTypes...>>();
        }
    }
}