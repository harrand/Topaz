#if TOPAZ_VULKAN
#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/os/window.hpp"

#ifdef _WIN32
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#else
#error could not decipher platform in vulkan rhi implementation
#endif
#include "vulkan/vulkan.h"
#include <vector>

namespace tz::gpu
{
	VkInstance current_instance = VK_NULL_HANDLE;
	VkDevice current_device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	hardware current_hardware;
	#define VULKAN_API_VERSION_USED VK_MAKE_API_VERSION(0, 1, 2, 0)

	const char* validation_layers[] =
	{
		#if TOPAZ_DEBUG
		"VK_LAYER_KHRONOS_validation"
		#endif
	};
	const char* layers[] =
	{
		#if TOPAZ_DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		#endif
	};

	/////////////////// chunky impl predecls ///////////////////
	#define ERROR_UNLESS_INIT if(current_instance == VK_NULL_HANDLE) return tz::error_code::precondition_failure;
	#define ASSERT_INIT tz_assert(current_instance != VK_NULL_HANDLE, "Topaz has not been initialised.");
	void impl_retrieve_physical_device_info(VkPhysicalDevice from, hardware& to);
	unsigned int impl_rate_hardware(const hardware&);
	// call this when you have need of the swapchain.
	// returns success if a swapchain already exists and is ready for use.
	// returns partial success if a new swapchain has been created (typically if this is your first time calling it, or if the old swapchain was out-of-date). you might want to rerecord commands.
	// returns partial_success if the swapchain previously existed, but has been recreated for some important reason (you will maybe need to rerecord commands)
	// returns oom if oom, voom if voom
	// returns unknown_error if some undocumented vulkan error occurred.
	tz::error_code impl_need_swapchain();

	/////////////////// tz::gpu api ///////////////////
	void initialise(tz::appinfo info)
	{
		// create a valid vulkan instance
		VkApplicationInfo vk_appinfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = info.name,
			.applicationVersion = VK_MAKE_API_VERSION(0, info.major, info.minor, 0),
			// todo: don't hardcode this idiot -.-
			.engineVersion = VK_MAKE_API_VERSION(0, 5, 0, 0),
			.apiVersion = VULKAN_API_VERSION_USED
		};
		VkInstanceCreateInfo inst_create
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &vk_appinfo,
			.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]),
			.ppEnabledLayerNames = validation_layers,
			.enabledExtensionCount = sizeof(layers) / sizeof(layers[0]),
			.ppEnabledExtensionNames = layers
		};
		VkResult res = vkCreateInstance(&inst_create, nullptr, &current_instance);	
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("OOM'd while initialising vulkan instance. Reduce memory usage and try again.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("VOOM'd while initialising vulkan instance. Reduce video memory usage and try again.");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				tz_error("Vulkan instance creation failed due to an implementation-specific error. Verify that your machine meets the minimum requirements, and proceed with troubleshooting.");
			break;
			case VK_ERROR_LAYER_NOT_PRESENT:
				tz_error("Vulkan instance creation failed due to a missing layer that the implementation asked for. Please submit a bug report.");
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz_error("Vulkan instance creation failed due to a missing extension that the implementation asked for. Verify that your machine meets the minimum requirements, and proceed with troubleshooting.");
			break;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				tz_error("Vulkan instance creation failed due to the requested version not being compatible by this machine's drivers. Verify that your machine meets the minimum requirements, and then update your graphics card drivers and try again.");
			break;
			default:
				tz_error("Vulkan instance creation failed due to an undocumented vulkan error code \"{}\"", static_cast<int>(res));
			break;
		}
	}

	void terminate()
	{
		if(current_device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(current_device, nullptr);
		}
		tz_assert(current_instance != VK_NULL_HANDLE, "Requested to terminate tz::gpu (vulkan) when the vulkan instance was null, implying we had never initialised. This is a game-side logic error.");
		vkDestroyInstance(current_instance, nullptr);
	}

	error_code iterate_hardware(std::span<hardware> devices, std::size_t* device_count)
	{
		if(device_count != nullptr)
		{
			*device_count = 0;
		}
		ERROR_UNLESS_INIT;
		// we need a buffer of vulkan native handles of size equal to the span provided, so we can fill the infos as necessary.
		std::vector<VkPhysicalDevice> vk_devices;
		vk_devices.resize(devices.size());

		std::uint32_t vk_device_count = devices.size();
		VkResult res = vkEnumeratePhysicalDevices(current_instance, &vk_device_count, vk_devices.data());
		if(device_count != nullptr)
		{
			*device_count = vk_device_count;
		}
		if(devices.empty())
		{
			return error_code::partial_success;
		}
		error_code ret;

		switch(res)
		{
			case VK_SUCCESS:
				ret = error_code::success;
			break;
			case VK_INCOMPLETE:
				ret = error_code::partial_success;
			break;
			default:
				ret = error_code::unknown_error;
			break;
		}

		for(std::size_t i = 0; i < std::min(devices.size(), static_cast<std::size_t>(vk_device_count)); i++)
		{
			hardware& dev = devices[i];
			VkPhysicalDevice vk_dev = vk_devices[i];
			impl_retrieve_physical_device_info(vk_dev, dev);
			dev.internals.i0 = static_cast<tz::hanval>(reinterpret_cast<std::uintptr_t>(vk_dev));
		}
		return ret;
	}

	hardware find_best_hardware()
	{
		std::size_t hardware_count;
		error_code res;
		std::vector<tz::gpu::hardware> hardware;
		hardware.resize(8);
		res = tz::gpu::iterate_hardware(hardware, &hardware_count);
		if(res == error_code::partial_success)
		{
			hardware.resize(hardware_count);
			res = tz::gpu::iterate_hardware(hardware);
		}
		else
		{
			tz_assert(res == error_code::success, "{} occurred when attempting to find the best hardware.", tz::error_code_name(res));
		}
		std::vector<unsigned int> hardware_scores(hardware_count);
		tz_assert(res == error_code::success, "find_best_hardware failed due to {}", tz::error_code_name(res));

		unsigned int max_score = 0;
		tz::gpu::hardware best_hardware = hardware.front();
		for(std::size_t i = 0; i < hardware_count; i++)
		{
			unsigned int score = impl_rate_hardware(hardware[i]);
			if(score > max_score)
			{
				max_score = score;
				best_hardware = hardware[i];
			}
		}
		return best_hardware;
	}

	error_code use_hardware(hardware hw)
	{
		if(hw.caps != hardware_capabilities::graphics_compute)
		{
			// incompatible hardware.
			return error_code::hardware_unsuitable;
		}
		auto pdev = reinterpret_cast<VkPhysicalDevice>(static_cast<std::uintptr_t>(hw.internals.i0.peek()));

		float queue_priority = 1.0f;
		VkDeviceQueueCreateInfo qcreate
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = hw.internals.i1,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority
		};

		VkPhysicalDeviceVulkan13Features features13
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = nullptr,
			.synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
		};
		VkPhysicalDeviceVulkan12Features features12
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &features13,
			.descriptorIndexing = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE,
		};
		VkPhysicalDeviceFeatures2 enabled_features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features12,
		};

		VkDeviceCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &enabled_features,
			.flags = 0,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &qcreate,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = 0,
			.ppEnabledExtensionNames = nullptr,
			.pEnabledFeatures = nullptr
		};

		VkResult res = vkCreateDevice(pdev, &create, nullptr, &current_device);
		current_hardware = hw;
		switch(res)
		{
			case VK_SUCCESS:
				return error_code::success;
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				return error_code::oom;
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				return error_code::voom;
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				return error_code::hardware_unsuitable;
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				switch(hw.features)
				{
					case tz::gpu::hardware_feature_coverage::ideal:
						return error_code::engine_bug;
					break;
					default:
						return error_code::hardware_unsuitable;
					break;
				}
				return error_code::precondition_failure;
			break;
			case VK_ERROR_DEVICE_LOST:
				return error_code::driver_hazard;
			break;
			default:
				return error_code::unknown_error;
			break;
		}
	}

	hardware get_used_hardware()
	{
		tz_assert(current_device != VK_NULL_HANDLE, "A call to get_used_hardware() was invalid because a piece of hardware has seemingly not yet been selected in a previous call to use_hardware(...). Did you forget to use a specific hardware component?");
		return current_hardware;
	}

	/////////////////// chunky impl IMPLEMENTATION ///////////////////

	void impl_retrieve_physical_device_info(VkPhysicalDevice from, hardware& to)
	{
		// get all the damn information about the stupid ass hardware, in as many lines as possible because this is vulkan
		VkPhysicalDeviceVulkan13Features features13
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = nullptr,
		};
		VkPhysicalDeviceVulkan12Features features12
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &features13,
		};
		VkPhysicalDeviceFeatures2 base_features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features12
		};
		vkGetPhysicalDeviceFeatures2(from, &base_features);
		VkPhysicalDeviceProperties2 base_properties
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
			.pNext = nullptr
		};
		vkGetPhysicalDeviceProperties2(from, &base_properties);
		VkPhysicalDeviceMemoryProperties base_memory;
		vkGetPhysicalDeviceMemoryProperties(from, &base_memory);
		to.vram_size_mib = 0;
		to.features = hardware_feature_coverage::insufficient;

		std::size_t missing_features = 0;
		if(!base_features.features.multiDrawIndirect)
		{
			missing_features++;
		}
		if(!features13.dynamicRendering)
		{
			missing_features++;
		}
		if(!features13.synchronization2)
		{
			missing_features++;
		}
		if(!features12.bufferDeviceAddress)
		{
			missing_features++;
		}
		if(!features12.descriptorIndexing)
		{
			missing_features++;
		}
		switch(missing_features)
		{
			case 0:
				to.features = hardware_feature_coverage::ideal;
			break;
			case 1:
			[[fallthrough]];
			case 2:
				to.features = hardware_feature_coverage::insufficient;	
			break;
			default:
				to.features = hardware_feature_coverage::poor;
			break;
		}
		
		switch(base_properties.properties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				to.type = tz::gpu::hardware_type::gpu;
			break;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				to.type = tz::gpu::hardware_type::integrated_gpu;
			break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				to.type = tz::gpu::hardware_type::cpu;
			break;
			default:
				to.type = tz::gpu::hardware_type::unknown;
			break;
		}
		to.name = base_properties.properties.deviceName;
		for(std::size_t i = 0; i < base_memory.memoryHeapCount; i++)
		{
			const VkMemoryHeap& heap = base_memory.memoryHeaps[i];
			if(heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			{
				to.vram_size_mib = std::max(to.vram_size_mib, heap.size / (1024 * 1024));
			}
		}

		// queue family ridiculous boilerplate. this is stupid.
		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(from, &family_count, nullptr);

		std::vector<VkQueueFamilyProperties> families(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(from, &family_count, families.data());
		for(std::size_t i = 0; i < family_count; i++)
		{
			const auto& fam = families[i];
			if(fam.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
			{
				to.caps = hardware_capabilities::graphics_compute;
				to.internals.i1 = i;
			}
			else if(fam.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				to.caps = hardware_capabilities::graphics_only;
				to.internals.i1 = i;
			}
			else if(fam.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				to.caps = hardware_capabilities::compute_only;
				to.internals.i1 = i;
			}
			else
			{
				to.caps = hardware_capabilities::neither;
			}
		}
	}

	unsigned int impl_rate_hardware(const hardware& hw)
	{
		// give the hardware a score based on how good it will be for rendering.
		unsigned int score = 0;
		switch(hw.caps)
		{
			case tz::gpu::hardware_capabilities::graphics_compute:
				// if it has graphics+compute in the same queue (this includes transfer), then i like it. otherwise it's dogshit.
				score += 200;
			break;
			default: break;
		}
		switch(hw.type)
		{
			// gpu >> igpu > anything else
			case tz::gpu::hardware_type::gpu:
				score += 1000;
			break;
			case tz::gpu::hardware_type::integrated_gpu:
				score += 250;
			break;
			default: break;
		}
		// give extra score based on the amount of memory of the largest heap on the hardware.
		// or in other words, more vram = good
		// arbitrarily: 6 GiB VRAM should be equivalent to 50 score.
		// i.e score = vram * 50 / (6 * 1024 * 1024 * 1024)
		score += hw.vram_size_mib * 50 / (6ull * 1024);
		switch(hw.features)
		{
			case tz::gpu::hardware_feature_coverage::ideal:
				// if it supports all features, then big score bonus.
				score += 500;
			break;
			case tz::gpu::hardware_feature_coverage::insufficient:
			[[fallthrough]];
			case tz::gpu::hardware_feature_coverage::poor:
				// if it is poor, then no matter what its specs are we really shouldnt use it.
				score = 0;
			break;
			default: break;
		}
		return score;
	}

	tz::error_code impl_need_swapchain()
	{
		tz::os::window_handle wnd = tz::os::get_window_handle();
		if(wnd == tz::nullhand)
		{
			return tz::error_code::precondition_failure;
		}
		if(surface == VK_NULL_HANDLE)
		{
			// create surface jit
			VkResult res;
			#ifdef _WIN32
				VkWin32SurfaceCreateInfoKHR create
				{
					.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
					.pNext = nullptr,
					.flags = 0
				};
				res = vkCreateWin32SurfaceKHR(current_instance, &create, nullptr, &surface);
			#else
				// assume xlib
				// 2 choices here for future harry:
				// a.) have the handle actually represent a pointer to some stable struct defined in impl_linux, containing both the display and the window and deref it here.
				// b.) if applicable, open a new display here if its easy and trivial to do, and just use that and let the handle represent the window?
				VkXlibSurfaceCreateInfoKHR create
				{
					.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
					.pNext = nullptr,
					.flags = 0,
					.dpy = 0,
					.window = 0,
				};
				#error swapchain creation for this platform is NYI
			#endif

			switch(res)
			{
				case VK_SUCCESS: break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					return tz::error_code::voom;
				break;
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					return tz::error_code::oom;
				break;
				default:
					return tz::error_code::unknown_error;
				break;
			}
		}
		// create the swapchain if need be.
		if(swapchain == VK_NULL_HANDLE)
		{
			return tz::error_code::success;
		}
		// recreate swapchain.
		tz_error("creating swapchain from valid surface {} is NYI", reinterpret_cast<std::uintptr_t>(surface));
		return tz::error_code::partial_success;
	}
}
#endif