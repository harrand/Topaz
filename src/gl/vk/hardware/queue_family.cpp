#if TZ_VULKAN
#include "gl/vk/hardware/queue_family.hpp"

namespace tz::gl::vk::hardware
{
    std::optional<VkQueueFamilyProperties> supports_queue_flag(std::span<const VkQueueFamilyProperties> fam_props, VkQueueFlagBits flag_type)
    {
        std::optional<VkQueueFamilyProperties> maybe_props = std::nullopt;
        for(const VkQueueFamilyProperties& cur_props : fam_props)
        {
            if(cur_props.queueFlags & flag_type)
            {
                maybe_props = cur_props;
                break;
            }
        }
        return maybe_props;
    }

    bool supports_queue_flags(std::span<const VkQueueFamilyProperties> fam_props, std::span<const VkQueueFlagBits> flag_types)
    {
        std::optional<VkQueueFamilyProperties> maybe_props = std::nullopt;
        bool result = true;
        for(const VkQueueFlagBits& flag_type : flag_types)
        {
            result &= supports_queue_flag(fam_props, flag_type).has_value();
        }
        
        return result;
    }
}

#endif