#ifndef TOPAZ_GL_VK_HARDWARE_MEMORY_HPP
#define TOPAZ_GL_VK_HARDWARE_MEMORY_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include "core/containers/basic_list.hpp"
#include "core/containers/enum_field.hpp"
#include <vector>
#include <cstdint>

namespace tz::gl::vk::hardware
{
    class Device;

    using MemoryTypeIndex = std::uint32_t;
    using HeapIndex = std::uint32_t;

    enum class MemoryType
    {
        DeviceLocal = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        HostVisible = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        HostCoherent = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        HostCached = VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    };

    enum class MemoryResidency
    {
        CPU,
        CPUPersistent,
        GPU
    };

    using MemoryTypeField = tz::EnumField<MemoryType>;

    struct MemoryModule
    {
        const Device* device;
        MemoryTypeField types;
        MemoryTypeIndex index;
        HeapIndex heap_index;

        bool operator==(const MemoryModule& mod) const = default;
    };

    class MemoryProperties
    {
    public:
        MemoryProperties(const Device& device);
        auto begin() const;
        auto end() const;
        tz::BasicList<MemoryModule> get_modules_matching(MemoryTypeField field) const;
        MemoryModule unsafe_get_some_module_matching(MemoryTypeField field) const;
    private:
        static MemoryTypeField parse_field(VkMemoryPropertyFlags flags);
        std::vector<MemoryModule> modules;
    };
}

#endif // TZ_VULKAN
#endif // TOPAZ_GL_VK_HARDWARE_MEMORY_HPP