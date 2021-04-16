#ifndef TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP
#define TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP
#if TZ_VULKAN
#include "vulkan/vulkan.h"
#include <optional>
#include <span>

namespace tz::gl::vk::hardware
{
    std::optional<VkQueueFamilyProperties> supports_queue_flag(std::span<const VkQueueFamilyProperties> fam_props, VkQueueFlagBits flag_type);
    bool supports_queue_flags(std::span<const VkQueueFamilyProperties> fam_props, std::span<const VkQueueFlagBits> flag_types);
}

#endif
#endif // TOPAZ_GL_VK_HARDWARE_QUEUE_FAMILY_HPP