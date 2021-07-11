#include "gl/impl/backend/vk/hardware/queue_family.hpp"
#include <array>

namespace tz::gl::vk::hardware
{
    template<VkPhysicalDeviceType Type>
    bool DeviceTypeFilter<Type>::satisfies(const hardware::Device& device) const
    {
        return device.get_properties().deviceType == Type;
    }
}