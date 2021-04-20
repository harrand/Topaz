#ifndef TOPAZ_GL_VK_VALIDATION_DEBUG_MESSENGER_HPP
#define TOPAZ_GL_VK_VALIDATION_DEBUG_MESSENGER_HPP
#if TZ_VULKAN
#include "gl/vk/impl/validation/debug_callback.hpp"

namespace tz::gl::vk::validation
{
    class DebugMessenger
    {
    public:
        DebugMessenger(VkInstance instance, VkDebugUtilsMessageSeverityFlagsEXT severity_filter, VkDebugUtilsMessageTypeFlagsEXT message_type_filter, VulkanDebugCallback auto callback, void* user_data);
        ~DebugMessenger();
        VkDebugUtilsMessengerEXT messenger;
        VkInstance current_instance;
    };

    void initialise_default_messenger(VkInstance instance);
    void destroy_default_messenger();
    DebugMessenger& get_default_messenger();

    VkResult vkext_create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT& debug_messenger);
    void vkext_destroy_debug_utils_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator);
}

#include "gl/vk/impl/validation/debug_messenger.inl"
#endif
#endif // TOPAZ_GL_VK_VALIDATION_DEBUG_MESSENGER_HPP