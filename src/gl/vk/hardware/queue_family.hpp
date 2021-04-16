#ifndef TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP
#define TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include <optional>
#include <span>
#include <vector>
#include <initializer_list>

namespace tz::gl::vk::hardware
{
    enum class QueueFamilyType
    {
        Graphics = VK_QUEUE_GRAPHICS_BIT,
        Compute = VK_QUEUE_COMPUTE_BIT,
        Transfer = VK_QUEUE_TRANSFER_BIT,
        SparseBinding = VK_QUEUE_SPARSE_BINDING_BIT
    };

    constexpr QueueFamilyType family_types[] = {QueueFamilyType::Graphics, QueueFamilyType::Compute, QueueFamilyType::Transfer, QueueFamilyType::SparseBinding};

    class QueueFamilyTypeField
    {
    public:
        QueueFamilyTypeField(std::initializer_list<QueueFamilyType> types = {});
        QueueFamilyTypeField& operator|=(QueueFamilyType type);
        QueueFamilyTypeField operator|(QueueFamilyType type) const;
        bool contains(QueueFamilyType type) const;
        bool contains(QueueFamilyTypeField field) const;
    private:
        std::vector<QueueFamilyType> supported_types;
    };

    using QueueFamilyIndex = int;

    struct DeviceQueueFamily
    {
        VkPhysicalDevice dev;
        QueueFamilyIndex index;
        QueueFamilyTypeField types_supported;
    };
}

#endif
#endif // TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP