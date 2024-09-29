#if TOPAZ_VULKAN
#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"
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

// vma refuses to fix compiler warnings, so use a nasty hack for each compiler (this will almost certainly require manual intervention for each of the big 3 -.-)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"

#define VMA_NOT_NULL
#define VMA_NULLABLE
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#pragma clang diagnostic pop
#include <vector>
#include <variant>

namespace tz::gpu
{
	VkInstance current_instance = VK_NULL_HANDLE;
	VkDevice current_device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	unsigned int swapchain_width = -1; unsigned int swapchain_height = -1;
	std::vector<VkImage> swapchain_images = {};
	hardware current_hardware;
	#define VULKAN_API_VERSION_USED VK_API_VERSION_1_3
	constexpr VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
	VmaAllocator alloc = VK_NULL_HANDLE;
	VkQueue graphics_compute_queue = VK_NULL_HANDLE;

	struct frame_data_t
	{
		VkCommandPool cpool = VK_NULL_HANDLE;
		VkCommandBuffer cmds = VK_NULL_HANDLE;
	};
	constexpr std::size_t frame_overlap = 2;
	std::array<frame_data_t, frame_overlap> frames;

	std::size_t global_resource_counter = 0;
	using generic_resource = std::variant<std::monostate, buffer_info, image_info>;
	struct resource_info
	{
		generic_resource res;	
		VkBuffer buf = VK_NULL_HANDLE;
		VkImage img = VK_NULL_HANDLE;
		VmaAllocation mem = VK_NULL_HANDLE;
		std::vector<std::byte> data;

		bool is_invalid() const
		{
			return res.index() == 0;
		}
		bool is_buffer() const
		{
			return res.index() == 1;
		}
		bool is_image() const
		{
			return res.index() == 2;
		}
	};
	std::vector<resource_info> resources = {};

	const char* validation_layers[] =
	{
		#if TOPAZ_DEBUG
		"VK_LAYER_KHRONOS_validation"
		#endif
	};
	const char* instance_extensions[] =
	{
		#if TOPAZ_DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		#endif
		VK_KHR_SURFACE_EXTENSION_NAME,
	};
	const char* device_extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
	tz::error_code impl_need_swapchain(std::uint32_t w, std::uint32_t h);
	VkFormat impl_get_format_from_image_type(tz::gpu::image_type type);

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
			.enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]),
			.ppEnabledExtensionNames = instance_extensions
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
		for(std::size_t i = 0 ; i < resources.size(); i++)
		{
			if(!resources[i].is_invalid())
			{
				destroy_resource(static_cast<tz::hanval>(i));
			}
		}
		if(swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(current_device, swapchain, nullptr);
			swapchain = VK_NULL_HANDLE;
		}
		if(alloc != VK_NULL_HANDLE)
		{
			vmaDestroyAllocator(alloc);
			alloc = VK_NULL_HANDLE;
		}
		vkDeviceWaitIdle(current_device);
		for(std::size_t i = 0; i < frame_overlap; i++)
		{
			if(frames[i].cpool != VK_NULL_HANDLE)
			{
				vkDestroyCommandPool(current_device, frames[i].cpool, nullptr);
			}
		}
		if(current_device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(current_device, nullptr);
			current_device = VK_NULL_HANDLE;
		}
		if(surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(current_instance, surface, nullptr);
			surface = VK_NULL_HANDLE;
		}
		tz_assert(current_instance != VK_NULL_HANDLE, "Requested to terminate tz::gpu (vulkan) when the vulkan instance was null, implying we had never initialised. This is a game-side logic error.");
		vkDestroyInstance(current_instance, nullptr);
		current_instance = VK_NULL_HANDLE;
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
			.enabledExtensionCount = sizeof(device_extensions) / sizeof(device_extensions[0]),
			.ppEnabledExtensionNames = device_extensions,
			.pEnabledFeatures = nullptr
		};

		VkResult res = vkCreateDevice(pdev, &create, nullptr, &current_device);
		current_hardware = hw;

		switch(res)
		{
			case VK_SUCCESS: break;
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

		vkGetDeviceQueue(current_device, current_hardware.internals.i1, 0, &graphics_compute_queue);

		VmaVulkanFunctions vk_funcs = {};
		vk_funcs.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vk_funcs.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo alloc_create
		{
			.flags = 0,
			.physicalDevice = pdev,
			.device = current_device,
			.pVulkanFunctions = &vk_funcs,
			.instance = current_instance,
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};
		res = vmaCreateAllocator(&alloc_create, &alloc);
		if(res != VK_SUCCESS)
		{
			return tz::error_code::unknown_error;
		}

		VkCommandPoolCreateInfo pool_create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = current_hardware.internals.i1
		};
		for(std::size_t i = 0; i < frame_overlap; i++)
		{
			VkResult res = vkCreateCommandPool(current_device, &pool_create, nullptr, &frames[i].cpool);
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

			VkCommandBufferAllocateInfo cmd_info
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = frames[i].cpool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			res = vkAllocateCommandBuffers(current_device, &cmd_info, &frames[i].cmds);
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

		return tz::error_code::success;
	}

	hardware get_used_hardware()
	{
		tz_assert(current_device != VK_NULL_HANDLE, "A call to get_used_hardware() was invalid because a piece of hardware has seemingly not yet been selected in a previous call to use_hardware(...). Did you forget to use a specific hardware component?");
		return current_hardware;
	}

	std::expected<resource_handle, tz::error_code> create_buffer(buffer_info info)
	{
		auto hanval = resources.size();
		resource_info& res = resources.emplace_back();
		res.res = info;

		VkBufferCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};
		(void)create;
		(void)hanval;
		return std::unexpected(tz::error_code::engine_bug);
	}

	std::expected<resource_handle, tz::error_code> create_image(image_info info)
	{
		auto hanval = resources.size();
		resource_info& res = resources.emplace_back();
		res.res = info;

		VkImageCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = impl_get_format_from_image_type(info.type),
			.extent =
			{
				.width = info.width,
				.height = info.height,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = info.access == tz::gpu::resource_access::static_access ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
			.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &current_hardware.internals.i1,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		VmaAllocationCreateInfo alloc_info = {};
		switch(info.access)
		{
			case tz::gpu::resource_access::static_access:
				alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
			case tz::gpu::resource_access::dynamic_access:
				alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
				alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			break;
		}

		VkResult ret = vmaCreateImage(alloc, &create, &alloc_info, &res.img, &res.mem, nullptr);
		switch(ret)
		{
			case VK_SUCCESS:
			break;
			default:
				return std::unexpected(tz::error_code::precondition_failure);
			break;
		}

		res.data.resize(info.data.size());
		std::copy(info.data.begin(), info.data.end(), res.data.begin());
		return static_cast<tz::hanval>(hanval);
	}

	tz::error_code destroy_resource(resource_handle res)
	{
		auto& info = resources[res.peek()];
		if(info.is_invalid())
		{
			return tz::error_code::precondition_failure;
		}
		else if(info.is_buffer())
		{
			// todo: implement
			return tz::error_code::engine_bug;
		}
		else if(info.is_image())
		{
			vmaDestroyImage(alloc, info.img, info.mem);
			info = {};
		}
		return tz::error_code::success;
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
				score += 200;
			break;
			// we dont support anything that doesnt support graphics+compute on a single queue.
			default: return 0; break;
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
			default: score += 1; break;
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
				return 0;
			break;
			default: break;
		}
		return score;
	}

	tz::error_code impl_need_swapchain(std::uint32_t w, std::uint32_t h)
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
		if(swapchain_width == w && swapchain_height == h)
		{
			// width/height are the same as before, no need to recreate swapchain.
			return tz::error_code::success;
		}
		// recreate swapchain.
		VkSwapchainCreateInfoKHR create
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = surface,
			.minImageCount = 2,
			.imageFormat = swapchain_format,
			.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			.imageExtent = {.width = w, .height = h},
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &current_hardware.internals.i1,
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
			.clipped = VK_FALSE,
			.oldSwapchain = swapchain
		};
		VkResult res = vkCreateSwapchainKHR(current_device, &create, nullptr, &swapchain);
		switch(res)
		{
			case VK_SUCCESS: break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				return tz::error_code::oom;
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				return tz::error_code::voom;
			break;
			case VK_ERROR_DEVICE_LOST:
				return tz::error_code::driver_hazard;
			break;
			case VK_ERROR_SURFACE_LOST_KHR:
				return tz::error_code::driver_hazard;
			break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				return tz::error_code::precondition_failure;
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				return tz::error_code::hardware_unsuitable;
			break;
			default:
				return tz::error_code::unknown_error;
			break;
		}
		swapchain_width = w;
		swapchain_height = h;

		swapchain_images.clear();
		std::uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(current_device, swapchain, &swapchain_image_count, nullptr);
		swapchain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(current_device, swapchain, &swapchain_image_count, swapchain_images.data());
		return tz::error_code::partial_success;
	}

	VkFormat impl_get_format_from_image_type(tz::gpu::image_type type)
	{
		switch(type)
		{
			case tz::gpu::image_type::rgba:
				return VK_FORMAT_R8G8B8A8_UNORM;
			break;
			case tz::gpu::image_type::depth:
				return VK_FORMAT_D16_UNORM;
			break;
			case tz::gpu::image_type::floats:
				return VK_FORMAT_R32_SFLOAT;
			break;
			default:
				return VK_FORMAT_UNDEFINED;
			break;
		}
	}
}
#endif