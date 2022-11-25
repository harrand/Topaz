#include "impl/vulkan/tzvk.hpp"
#include "hdk/debug.hpp"

#include "volk.h"

#include <algorithm>
#include <span>
#include <limits>
#include <optional>

constexpr std::uint32_t req_vk_version = VK_VERSION_1_3;

namespace tz::impl_vk
{
	struct system_t
	{
		bool initialise_attempted = false;
		VkInstance instance = VK_NULL_HANDLE;
		std::vector<VkPhysicalDevice> installed_devices = {};
		VkPhysicalDevice selected_device = VK_NULL_HANDLE;
		std::uint32_t render_queue_family_index = std::numeric_limits<std::uint32_t>::max();
		VkDevice render_device = VK_NULL_HANDLE;
		
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

		std::optional<std::uint32_t> get_render_queue_family_index(VkPhysicalDevice device)
		{
			// TODO: Check for present aswell.	
			std::uint32_t family_count;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
			std::vector<VkQueueFamilyProperties> props(family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, props.data());
			auto iter = std::find_if(props.begin(), props.end(), [](VkQueueFamilyProperties prop)
			{
				return prop.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
			});
			if(iter != props.end())
			{
				return {std::distance(props.begin(), iter)};
			}
			return std::nullopt;
		}

		bool is_physical_device_appropriate(VkPhysicalDevice dev)
		{
			return get_render_queue_family_index(dev).has_value();
		}

		VkPhysicalDevice select_physical_device(std::span<VkPhysicalDevice> devices)
		{
			hdk::assert(!devices.empty(), "vulkan: attempting to select physical device, but the provided span is empty. bug report needed.");
			// We want to ignore all devices for which we cant create a fitting VkDevice from to satisfy our requirements.
			auto valid_end = std::remove_if(devices.begin(), devices.end(), [](VkPhysicalDevice dev)
			{
				return !is_physical_device_appropriate(dev);
			});
			// simply retrieve highest rated device.
			return *std::max_element(devices.begin(), valid_end, [](VkPhysicalDevice a, VkPhysicalDevice b){return rate_physical_device(a) < rate_physical_device(b);});
		}
	
		VkDevice system_make_render_device()
		{
			// First make sure we've actually selected a valid physical device.
			hdk::assert(system.render_device == VK_NULL_HANDLE, "Attempted to create vulkan device but it seems to already have been made. Logic error.");
			hdk::assert(system.selected_device != VK_NULL_HANDLE && is_physical_device_appropriate(system.selected_device), "Attempting to create vulkan device, but the selected physical device is not appropriate. Either logic error or your machine is not compatible with topaz.");
			{
				auto maybe_queue_family_index = get_render_queue_family_index(system.selected_device);
				hdk::assert(maybe_queue_family_index.has_value(), "Physical device was considered to be valid, but we still can't retrieve a queue family index for our needs -- Logic error.");
				system.render_queue_family_index = maybe_queue_family_index.value();
			}
			// We're only going to create a single queue for now.
			constexpr float default_priority = 1.0f;
			VkDeviceQueueCreateInfo queue_create
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = system.render_queue_family_index,
				.queueCount = 1,
				.pQueuePriorities = &default_priority
			};

			VkDeviceCreateInfo device_create
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &queue_create,
				.enabledLayerCount = 0,
				.ppEnabledLayerNames = nullptr,
				.enabledExtensionCount = 0,
				.ppEnabledExtensionNames = nullptr,
				.pEnabledFeatures = nullptr
			};
			VkDevice ret;
			VkResult res = vkCreateDevice(system.selected_device, &device_create, nullptr, &ret);	
			switch(res)
			{
				case VK_SUCCESS:
				break;
				case VK_ERROR_OUT_OF_HOST_MEMORY:
				[[fallthrough]];
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					hdk::error("Failed to create VkDevice due to oom.");
				break;
				case VK_ERROR_INITIALIZATION_FAILED:
					hdk::error("Vulkan backend not properly initialised by the time we try to create a VkDevice.");
				break;
				case VK_ERROR_EXTENSION_NOT_PRESENT:
					hdk::error("One or more VkDevice extensions specified weren't supported. We should've been checking for this beforehand. Logic error.");
				break;
				case VK_ERROR_FEATURE_NOT_PRESENT:
					hdk::error("One or more VkDevice features specified weren't supported. We should've been checking for this beforehand. Logic error.");
				break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					hdk::error("VkDevice creation failed due to too many objects. Logic error.");
				break;
				case VK_ERROR_DEVICE_LOST:
					hdk::error("VkDevice creation failed due to device lost. Either engine has a severe bug, or another application that was just running has crashed the GPU.");
				break;
			}
			return ret;
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
		system.render_device = detail::system_make_render_device();

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
		hdk::assert(system.render_device != VK_NULL_HANDLE);
		hdk::assert(system.instance != VK_NULL_HANDLE);

		vkDestroyDevice(system.render_device, nullptr);
		system.render_device = VK_NULL_HANDLE;
	
		vkDestroyInstance(system.instance, nullptr);
		system.instance = VK_NULL_HANDLE;

		system.initialise_attempted = false;
		hdk::report("Vulkan Terminated");
	}
}
