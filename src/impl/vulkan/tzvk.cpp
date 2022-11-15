#include "impl/vulkan/tzvk.hpp"
#include "hdk/debug.hpp"

#include "volk.h"

constexpr std::uint32_t req_vk_version = VK_VERSION_1_3;

namespace tz::impl_vk
{
	struct system_t
	{
		bool initialise_attempted = false;
		VkInstance instance = VK_NULL_HANDLE;
	} system;

	void initialise()
	{
		hdk::assert(!system.initialise_attempted, "Vulkan backend already initialise_attempted!");

		VkResult res = volkInitialize();
		hdk::assert(res == VK_SUCCESS, "`volkInitialize` failed. This means the vulkan loader is not installed on your system.");

		std::uint32_t inst_vk_version = volkGetInstanceVersion();
		hdk::assert(inst_vk_version >= req_vk_version, "Instance-level version does not support at least Vulkan %zu.%zu. It returned %zu.%zu", VK_API_VERSION_MAJOR(req_vk_version), VK_API_VERSION_MINOR(req_vk_version), VK_API_VERSION_MAJOR(inst_vk_version), VK_API_VERSION_MINOR(inst_vk_version));

		{
			constexpr const char* instance_layers[] =
			{
				#if HDK_DEBUG
				"VK_LAYER_KHRONOS_validation"
				#endif
			};

			constexpr const char* instance_extensions[] = {};

			VkApplicationInfo app_info
			{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = nullptr,
				.pApplicationName = "Topaz 4.0 Untitled Application",
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName = "Topaz 4.0",
				.apiVersion = req_vk_version
			};

			VkInstanceCreateInfo inst_create_info
			{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.pApplicationInfo = &app_info,
				.enabledLayerCount = sizeof(instance_layers) / sizeof(instance_layers[0]),
				.ppEnabledLayerNames = instance_layers,
				.enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]),
				.ppEnabledExtensionNames = instance_extensions
			};

			VkResult res = vkCreateInstance(&inst_create_info, nullptr, &system.instance);
			switch(res)
			{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
				[[fallthrough]];
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					hdk::error("OOM while creating vulkan instance");
				break;
				
			}
			hdk::assert(res == VK_SUCCESS, "Vulkan instance creation failed.");
			volkLoadInstance(system.instance);
			hdk::report("Vulkan Initialised");
		}

		system.initialise_attempted = true;
	}

	void terminate()
	{
		hdk::assert(system.initialise_attempted, "Vulkan backend was never initialise_attempted!");
	
		vkDestroyInstance(system.instance, nullptr);
		system.instance = VK_NULL_HANDLE;

		system.initialise_attempted = false;
		hdk::report("Vulkan Terminated");
	}
}
