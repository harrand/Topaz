#ifndef TOPAZ_GL_VK_VALIDATION_LAYER_HPP
#define TOPAZ_GL_VK_VALIDATION_LAYER_HPP
#if TZ_VULKAN
#include "core/containers/basic_list.hpp"
#include <string>

namespace tz::gl::vk
{
    namespace validation
    {
        #if TZ_DEBUG
        constexpr bool layers_enabled = true;
        #else
        constexpr bool layers_enabled = false;
        #endif
    }
    using VulkanValidationLayer = std::string;
    using ValidationLayerList = tz::BasicList<VulkanValidationLayer>;

    ValidationLayerList get_all_available_layers();
    ValidationLayerList get_default_validation_layers();
}

#endif
#endif // TOPAZ_GL_VK_VALIDATION_LAYER_HPP