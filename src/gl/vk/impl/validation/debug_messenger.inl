namespace tz::gl::vk::validation
{
    DebugMessenger::DebugMessenger
    (
        VkInstance instance,
        VkDebugUtilsMessageSeverityFlagsEXT severity_filter,
        VkDebugUtilsMessageTypeFlagsEXT message_type_filter,
        VulkanDebugCallback auto callback,
        void* user_data
    ):
    messenger(),
    current_instance(instance)
    {
        VkDebugUtilsMessengerCreateInfoEXT create{};
        create.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create.messageSeverity = severity_filter;
        create.messageType = message_type_filter;
        create.pfnUserCallback = callback;
        create.pUserData = nullptr;

        vkext_create_debug_utils_messenger(this->current_instance, create, nullptr, this->messenger);
    }
}