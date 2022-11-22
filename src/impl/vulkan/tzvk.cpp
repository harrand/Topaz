#include "impl/vulkan/tzvk.hpp"
#include "hdk/debug.hpp"

#include "volk.h"

#include <algorithm>
#include <span>

constexpr std::uint32_t req_vk_version = VK_VERSION_1_3;

namespace tz::impl_vk
{
	struct system_t
	{
		bool initialise_attempted = false;
		VkInstance instance = VK_NULL_HANDLE;
		std::vector<VkPhysicalDevice> installed_devices = {};
		VkPhysicalDevice selected_device = VK_NULL_HANDLE;
		
		void debug_check(){hdk::assert(this->instance != VK_NULL_HANDLE, "Vulkan Instance null, system_t invalid or not initialised.");}
	} system;

	namespace detail
	{
		void system_init_instance(const initialise_info_t& info)
		{
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
					.pApplicationName = info.app_name,
					.applicationVersion = VK_MAKE_VERSION(info.app_version.major, info.app_version.minor, info.app_version.patch),
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
			}
		}

		void system_enum_devices()
		{
			system.debug_check();
			std::uint32_t count;

			auto enum_dbgchk = [](VkResult res)
			{
				switch(res)
				{
					case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					[[fallthrough]];
					case VK_ERROR_OUT_OF_HOST_MEMORY:
						hdk::error("vulkan: oom while enumerating physical devices");
					break;
					case VK_ERROR_INITIALIZATION_FAILED:
						hdk::error("vulkan: vkEnumeratePhysicalDevices returned VK_ERROR_INITIALIZATION_FAILED, but instance seems correct. corruption?");
					break;
					case VK_INCOMPLETE:
						hdk::error("vulkan: could retrieve some, but not all of the physical devices installed on the machine. this is a logic error. bug report needed.");
					break;
					case VK_SUCCESS:

					break;
					default:
						hdk::error("vulkan: vkEnumeratePhysicalDevices returned undocumented error code (%#08X). no info available. submit bug report", res);
					break;
				}
			};

			VkResult res = vkEnumeratePhysicalDevices(system.instance, &count, nullptr);
			enum_dbgchk(res);
			system.installed_devices.resize(count);
			res = vkEnumeratePhysicalDevices(system.instance, &count, system.installed_devices.data());
			enum_dbgchk(res);
		}
	
		unsigned int rate_physical_device(VkPhysicalDevice dev)
		{
			unsigned int score = 0;
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(dev, &props);
			switch(props.deviceType)
			{
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					score += 500;
				break;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					score += 2000;
				break;
			}
			return score;
		}

		VkPhysicalDevice select_physical_device(std::span<const VkPhysicalDevice> devices)
		{
			hdk::assert(!devices.empty(), "vulkan: attempting to select physical device, but the provided span is empty. bug report needed.");
			// simply retrieve highest rated device.
			return *std::max_element(devices.begin(), devices.end(), [](VkPhysicalDevice a, VkPhysicalDevice b){return rate_physical_device(a) < rate_physical_device(b);});
		}
	}

	void initialise(initialise_info_t info)
	{
		hdk::assert(!system.initialise_attempted, "Vulkan backend already initialise_attempted!");

		VkResult res = volkInitialize();
		hdk::assert(res == VK_SUCCESS, "`volkInitialize` failed. This means the vulkan loader is not installed on your system.");
		detail::system_init_instance(info);
		detail::system_enum_devices();
		system.selected_device = detail::select_physical_device(system.installed_devices);

		#if HDK_DEBUG
		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(system.selected_device, &props);
			hdk::report("Vulkan Initialised\n\tGPU: %s (score: %u)", props.deviceName, detail::rate_physical_device(system.selected_device));
		}
		#endif

		system.initialise_attempted = true;
	}

	void initialise()
	{
		initialise
		({
			.app_version = hdk::version{1, 0, 0},
			.app_name = "Untitled Topaz Application"
		});
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
