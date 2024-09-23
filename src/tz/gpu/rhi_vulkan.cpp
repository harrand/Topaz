#if TOPAZ_VULKAN
#include "tz/topaz.hpp"
#include "tz/gpu/device.hpp"

#include "vulkan/vulkan.h"
#include <vector>
#include <array>

namespace tz::gpu
{
	VkInstance current_instance = VK_NULL_HANDLE;
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

	std::array<const char*, static_cast<int>(error_code::_count)> error_code_strings
	{
		"success",
		"partial success",
		"precondition failure error",
		"unknown error",
		"out of CPU memory error",
		"out of GPU memory error",
	};

	std::array<const char*, static_cast<int>(hardware_feature_coverage::_count)> hardware_feature_coverage_strings
	{
		"ideal",
		"insufficient",
		"poor"
	};

	/////////////////// chunky impl predecls ///////////////////
	void impl_retrieve_physical_device_info(VkPhysicalDevice from, hardware& to);
	unsigned int impl_rate_hardware(const hardware&);

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
		tz_assert(current_instance != VK_NULL_HANDLE, "Requested to terminate tz::gpu (vulkan) when the vulkan instance was null, implying we had never initialised. This is a game-side logic error.");
		vkDestroyInstance(current_instance, nullptr);
	}

	error_code iterate_hardware(std::span<hardware> devices, std::size_t* device_count)
	{
		if(device_count != nullptr)
		{
			*device_count = 0;
		}
		if(current_instance == VK_NULL_HANDLE)
		{
			return error_code::precondition_failure;
		}
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
		tz::gpu::error_code res;
		std::vector<tz::gpu::hardware> hardware;
		hardware.resize(8);
		res = tz::gpu::iterate_hardware(hardware, &hardware_count);
		if(res == error_code::partial_success)
		{
			hardware.resize(hardware_count);
			res = tz::gpu::iterate_hardware(hardware);
		}
		std::vector<unsigned int> hardware_scores(hardware_count);
		tz_assert(res == error_code::success, "find_best_hardware failed due to {}", error_code_strings[static_cast<int>(res)]);

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
		tz_assert(hw.caps != hardware_capabilities::neither, "call to use_hardware using hardware \"{}\" which supports neither 'graphics' nor 'compute' operations. you can only create a device from a piece of hardware that supports at least one of these.", hw.name);
		auto pdev = reinterpret_cast<VkPhysicalDevice>(static_cast<std::uintptr_t>(hw.internals.i0.peek()));

		float queue_priority = 1.0f;
		VkDeviceQueueCreateInfo qcreate
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = hw.internals.i1,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority
		};

		VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES ,
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingPartiallyBound = VK_TRUE,
			.descriptorBindingVariableDescriptorCount = VK_TRUE,
			.runtimeDescriptorArray = VK_TRUE,
		};
		VkPhysicalDeviceFeatures2 enabled_features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &descriptor_indexing_features
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

		VkDevice ldev;
		VkResult res = vkCreateDevice(pdev, &create, nullptr, &ldev);
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
			case VK_ERROR_INITIALIZATION_FAILED:
				return error_code::unknown_error;
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz_error("Vulkan device creation failed due to a missing extension that the implementation asked for. Verify that your machine meets the minimum requirements, and proceed with troubleshooting.");
				return error_code::precondition_failure;
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				switch(hw.features)
				{
					case tz::gpu::hardware_feature_coverage::ideal:
					tz_error("Vulkan device creation failed due to a missing device feature that the implementation asked for. This is surprising, as Topaz considered the feature coverage of \"{}\" to be: \"{}\". Please submit a bug report.", hw.name, hardware_feature_coverage_strings[static_cast<int>(hw.features)]);
					break;
					default:
					tz_error("Vulkan device creation failed due to a missing device feature that the implementation asked for. However, Topaz considered the feature coverage of \"{}\" to be: \"{}\". Your GPU does not meet Topaz's system requirements. Try updating your drivers and trying again.", hw.name, hardware_feature_coverage_strings[static_cast<int>(hw.features)]);
					break;
				}
				return error_code::precondition_failure;
			break;
			case VK_ERROR_TOO_MANY_OBJECTS:
				tz_error("Vulkan device creation failed due to too many objects. The driver says you have created too many devices. If you think this diagnosis is incorrect, proceed with troubleshooting.");
				return error_code::unknown_error;
			break;
			case VK_ERROR_DEVICE_LOST:
				tz_error("Vulkan device creation failed due to device lost. This could be a once-off crash. If not, please verify that your machine meets the minimum requirements, and proceed with troubleshooting.");
				return error_code::unknown_error;
			break;
			default:
				tz_error("GPU Device creation failed due to undocumented vulkan error \"{}\"", static_cast<int>(res));
				return error_code::unknown_error;
			break;
		}
	}

	void destroy_device(device_handle device)
	{
		auto ldev = reinterpret_cast<VkDevice>(static_cast<std::uintptr_t>(device.peek()));
		vkDestroyDevice(ldev, nullptr);
	}

	/////////////////// chunky impl IMPLEMENTATION ///////////////////

	void impl_retrieve_physical_device_info(VkPhysicalDevice from, hardware& to)
	{
		// get all the damn information about the stupid ass hardware, in as many lines as possible because this is vulkan
		VkPhysicalDeviceDescriptorIndexingFeatures descriptor_features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES ,
			.pNext = nullptr
		};
		VkPhysicalDeviceFeatures2 base_features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &descriptor_features
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
		if(!descriptor_features.runtimeDescriptorArray)
		{
			missing_features++;
		}
		if(!descriptor_features.descriptorBindingPartiallyBound)
		{
			missing_features++;
		}
		if(!descriptor_features.descriptorBindingVariableDescriptorCount)
		{
			missing_features++;
		}
		if(!descriptor_features.shaderSampledImageArrayNonUniformIndexing)
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
			case tz::gpu::hardware_feature_coverage::poor:
				// if it is poor, then no matter what its specs are we really shouldnt use it.
				score = 0;
			break;
			default: break;
		}
		return score;
	}
}
#endif