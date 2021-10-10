#if TZ_VULKAN
#include "gl/impl/backend/vk/validation/debug_messenger.hpp"

namespace tz::gl::vk::validation
{
	DebugMessenger::~DebugMessenger()
	{
		vkext_destroy_debug_utils_messenger(this->current_instance, this->messenger, nullptr);
	}
	
	DebugMessenger* def_msg = nullptr;

	void initialise_default_messenger(VkInstance instance)
	{
		tz_assert(def_msg == nullptr, "Double-call of initialise_default_messenger?");
		def_msg = new DebugMessenger
		(
			instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

			tz::gl::vk::validation::default_debug_callback,
			nullptr
		);
	}

	void destroy_default_messenger()
	{
		tz_assert(def_msg != nullptr, "Double-call of destroy_default_messenger()?");
		delete def_msg;
	}

	DebugMessenger& default_messenger()
	{
		tz_assert(def_msg != nullptr, "Retrieving default_messenger() without initialising it first!");
		return *def_msg;
	}

	VkResult vkext_create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT& create_info, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT& debug_messenger)
	{
		#if TZ_DEBUG
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if(func != nullptr)
		{
			return func(instance, &create_info, allocator, &debug_messenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
		#else
		return VK_ERROR_UNKNOWN;
		#endif
	}

	void vkext_destroy_debug_utils_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator)
	{
		#if TZ_DEBUG
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if(func != nullptr)
		{
			func(instance, debug_messenger, allocator);
		}
		#endif
	}

}
#endif // TZ_VULKAN