#ifndef TOPAZ_GL_VK_VALIDATION_DEBUG_CALLBACK_HPP
#define TOPAZ_GL_VK_VALIDATION_DEBUG_CALLBACK_HPP
#if TZ_VULKAN
#include "core/types.hpp"
#include "core/assert.hpp"
#include "vulkan/vulkan.h"

namespace tz::gl::vk
{
    namespace validation
    {
        template<typename F>
        concept VulkanDebugCallback = tz::Function<F,
            VKAPI_ATTR VkBool32, /*Return Type*/

            /*Argument Types*/
            VkDebugUtilsMessageSeverityFlagBitsEXT,
            VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT*,
            void*
            >;

        inline VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback
        (
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            [[maybe_unused]] void* user_data
        )
        {
            if(message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                // Warning or more severe. Let's error on it.
                tz_error("[Vulkan Debug Callback]: %s\n", callback_data->pMessage);
            }
            return VK_FALSE;
        }
    }
}

#endif
#endif //TOPAZ_GL_VK_VALIDATION_DEBUG_CALLBACK_HPP