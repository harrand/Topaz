#ifndef TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP
#define TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include "core/containers/enum_field.hpp"
#include <optional>
#include <span>
#include <vector>
#include <initializer_list>

namespace tz::gl::vk::hardware
{
    // Pre-defines
    class Device;

    enum class QueueFamilyType
    {
        Graphics,
        Compute,
        Transfer,
        SparseBinding,
        Present
    };

    using QueueFamilyTypeField = tz::EnumField<QueueFamilyType>;

    using QueueFamilyIndex = int;

    struct DeviceQueueFamily
    {
        const Device* dev;
        QueueFamilyIndex index;
        QueueFamilyTypeField types_supported;
    };
}

#endif
#endif // TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP