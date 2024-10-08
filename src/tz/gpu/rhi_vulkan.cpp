#if TOPAZ_VULKAN
#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/shader.hpp"
#include "tz/gpu/pass.hpp"
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
	VkPipelineLayout default_layout = VK_NULL_HANDLE;
	constexpr std::uint32_t max_image_count = 8192;

	VkCommandPool scratch_pool = VK_NULL_HANDLE;
	VkCommandBuffer scratch_cmds = VK_NULL_HANDLE;
	VkFence scratch_fence = VK_NULL_HANDLE;
	struct frame_data_t
	{
		VkCommandPool cpool = VK_NULL_HANDLE;
		VkCommandBuffer cmds = VK_NULL_HANDLE;

		// might not need it. signalled when image is acquired. dont think CPU ever needs to wait?
		VkFence swapchain_fence = VK_NULL_HANDLE;
		// signalled when image is acquired, ideally present operations will wait on this.
		VkSemaphore swapchain_sem = VK_NULL_HANDLE;
		// incremented for each "renderer" in the timeline. allows renderer dependencies to be sync'd properly.
		VkSemaphore timeline_sem = VK_NULL_HANDLE;
	};
	constexpr std::size_t frame_overlap = 2;
	std::array<VkDescriptorSetLayout, frame_overlap> set_layouts = {};
	std::array<frame_data_t, frame_overlap> frames;

	std::size_t global_resource_counter = 0;
	using generic_resource = std::variant<std::monostate, buffer_info, image_info>;
	struct resource_info
	{
		generic_resource res;	
		VkBuffer buf = VK_NULL_HANDLE;
		VkDeviceAddress buffer_device_address = 0;
		void* buffer_mapped_address = nullptr;
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
	enum class shader_type
	{
		vertex, fragment, compute
	};
	struct shader_info
	{
		shader_type ty;
		VkShaderModule smod;
	};
	std::vector<shader_info> shaders = {};
	struct pass_data
	{
		pass_info info;
		VkPipelineLayout layout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
	};
	std::vector<pass_data> passes = {};

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
		#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		#elif defined(__linux__)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
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
	VkPipelineLayout impl_create_layout();
	// call this when you have need of the swapchain.
	// returns success if a swapchain already exists and is ready for use.
	// returns partial success if a new swapchain has been created (typically if this is your first time calling it, or if the old swapchain was out-of-date). you might want to rerecord commands.
	// returns partial_success if the swapchain previously existed, but has been recreated for some important reason (you will maybe need to rerecord commands)
	// returns oom if oom, voom if voom
	// returns unknown_error if some undocumented vulkan error occurred.
	tz::error_code impl_need_swapchain(std::uint32_t w, std::uint32_t h);
	VkFormat impl_get_format_from_image_type(tz::gpu::image_type type);
	void impl_write_all_resources(pass_handle pass);
	void impl_record_gpu_work(pass_handle pass);

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
		passes.reserve(256);
		shaders.reserve(256);
	}

	void terminate()
	{
		for(std::size_t i = 0 ; i < resources.size(); i++)
		{
			if(!resources[i].is_invalid())
			{
				tz_must(destroy_resource(static_cast<tz::hanval>(i)));
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
		if(current_device != VK_NULL_HANDLE)
		{
			// destroy command buffers
			vkDeviceWaitIdle(current_device);
			if(scratch_pool != VK_NULL_HANDLE)
			{
				vkDestroyCommandPool(current_device, scratch_pool, nullptr);
				scratch_pool = VK_NULL_HANDLE;
			}
			if(scratch_fence != VK_NULL_HANDLE)
			{
				vkDestroyFence(current_device, scratch_fence, nullptr);
				scratch_fence = VK_NULL_HANDLE;
			}
			for(std::size_t i = 0; i < frame_overlap; i++)
			{
				if(frames[i].cpool != VK_NULL_HANDLE)
				{
					vkDestroyCommandPool(current_device, frames[i].cpool, nullptr);
					frames[i].cpool = VK_NULL_HANDLE;
					frames[i].cmds = VK_NULL_HANDLE;
				}
				if(frames[i].swapchain_fence != VK_NULL_HANDLE)
				{
					vkDestroyFence(current_device, frames[i].swapchain_fence, nullptr);
					frames[i].swapchain_fence = VK_NULL_HANDLE;
				}
				if(frames[i].swapchain_sem != VK_NULL_HANDLE)
				{
					vkDestroySemaphore(current_device, frames[i].swapchain_sem, nullptr);
					frames[i].swapchain_sem = VK_NULL_HANDLE;
				}
				if(frames[i].timeline_sem != VK_NULL_HANDLE)
				{
					vkDestroySemaphore(current_device, frames[i].timeline_sem, nullptr);
					frames[i].timeline_sem = VK_NULL_HANDLE;
				}
			}
			for(std::size_t i = 0; i < shaders.size(); i++)
			{
				if(shaders[i].smod != VK_NULL_HANDLE)
				{
					vkDestroyShaderModule(current_device, shaders[i].smod, nullptr);
					shaders[i] = {};
				}
			}
			for(std::size_t i = 0; i < passes.size(); i++)
			{
				if(passes[i].pipeline != VK_NULL_HANDLE)
				{
					vkDestroyPipeline(current_device, passes[i].pipeline, nullptr);
					passes[i] = {};
				}
			}
			if(default_layout != VK_NULL_HANDLE)
			{
				vkDestroyPipelineLayout(current_device, default_layout, nullptr);
				default_layout = VK_NULL_HANDLE;

				for(auto& set_layout : set_layouts)
				{
					vkDestroyDescriptorSetLayout(current_device, set_layout, nullptr);
					set_layout = VK_NULL_HANDLE;
				}
			}
			// then destroy the device itself.
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
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				RETERR(tz::error_code::oom, "ran out of CPU memory whilst iterating over hardware");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				RETERR(tz::error_code::voom, "ran out of GPU memory whilst iterating over hardware");
			break;
			default:
				RETERR(tz::error_code::unknown_error, "an implementation-side error occurred in the driver whilst iterating over hardware");
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
		std::vector<unsigned int> hardware_scores(hardware_count);
		tz_must(res);

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
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
			.descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
			.descriptorBindingPartiallyBound = VK_TRUE,
			.descriptorBindingVariableDescriptorCount = VK_TRUE,
			.runtimeDescriptorArray = VK_TRUE,
			.timelineSemaphore = VK_TRUE,
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
				RETERR(tz::error_code::oom, "ran out of CPU memory while trying to create vulkan device");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				RETERR(tz::error_code::voom, "ran out of GPU memory while trying to create vulkan device");
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				RETERR(tz::error_code::hardware_unsuitable, "the hardware requested for use is not suitable due to a missing vulkan extension");
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				switch(hw.features)
				{
					case tz::gpu::hardware_feature_coverage::ideal:
						RETERR(tz::error_code::engine_bug, "vulkan driver says not all features were available for the device, even though the engine said it was suitable. please submit a bug report.");
					break;
					default:
						RETERR(tz::error_code::hardware_unsuitable, "the hardware requested for use is not suitable due to a missing vulkan feature");
					break;
				}
			break;
			case VK_ERROR_DEVICE_LOST:
				RETERR(tz::error_code::driver_hazard, "vulkan driver reported a device lost error whilst trying to create a device. something extremely cursed has happened - time to troubleshoot!");
			break;
			default:
				RETERR(tz::error_code::unknown_error, "vulkan device creation failed due to an undocumented error.");
			break;
		}

		vkGetDeviceQueue(current_device, current_hardware.internals.i1, 0, &graphics_compute_queue);

		VmaVulkanFunctions vk_funcs = {};
		vk_funcs.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vk_funcs.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo alloc_create
		{
			.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice = pdev,
			.device = current_device,
			.pVulkanFunctions = &vk_funcs,
			.instance = current_instance,
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};
		res = vmaCreateAllocator(&alloc_create, &alloc);
		if(res != VK_SUCCESS)
		{
			RETERR(tz::error_code::unknown_error, "vulkan memory allocator creation failed due to an unknown error");
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
					RETERR(tz::error_code::voom, "ran out of GPU memory while trying to create a vulkan command pool");
				break;
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					RETERR(tz::error_code::oom, "ran out of CPU memory while trying to create a vulkan command pool");
				break;
				default:
					RETERR(tz::error_code::unknown_error, "undocumented error code reported while trying to create a vulkan command pool");
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
					RETERR(tz::error_code::voom, "ran out of GPU memory while trying to create vulkan command buffers");
				break;
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					RETERR(tz::error_code::oom, "ran out of CPU memory while trying to create vulkan command buffers");
				break;
				default:
					RETERR(tz::error_code::unknown_error, "undocumented error code reported while trying to create vulkan command buffers");
				break;
			}

			// Sync Variables
			VkFenceCreateInfo fence_create
			{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0
			};
			// todo: error check.
			vkCreateFence(current_device, &fence_create, nullptr, &frames[i].swapchain_fence);

			VkSemaphoreCreateInfo swapchain_sem_create
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0
			};
			// todo: error check.
			vkCreateSemaphore(current_device, &swapchain_sem_create, nullptr, &frames[i].swapchain_sem);

			VkSemaphoreTypeCreateInfo timeline_create
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.pNext = nullptr,
				.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
				.initialValue = 0
			};
			VkSemaphoreCreateInfo sem_create
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = &timeline_create,
				.flags = 0
			};
			// todo: error check.
			vkCreateSemaphore(current_device, &sem_create, nullptr, &frames[i].timeline_sem);
		}
		default_layout = impl_create_layout();

		vkCreateCommandPool(current_device, &pool_create, nullptr, &scratch_pool);
		VkCommandBufferAllocateInfo cmd_info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = scratch_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		res = vkAllocateCommandBuffers(current_device, &cmd_info, &scratch_cmds);

		VkFenceCreateInfo fence_create
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};
		vkCreateFence(current_device, &fence_create, nullptr, &scratch_fence);

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
		std::string_view name = info.debug_name;
		if(name.empty())
		{
			name = "unnamed buffer";
		}

		VkBufferCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = info.data.size_bytes(),
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &current_hardware.internals.i1
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
		VmaAllocationInfo alloc_result;
		VkResult ret = vmaCreateBuffer(alloc, &create, &alloc_info, &res.buf, &res.mem, &alloc_result);
		if(info.access == tz::gpu::resource_access::dynamic_access)
		{
			res.buffer_mapped_address = alloc_result.pMappedData;
		}
		VkBufferDeviceAddressInfo bda
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.pNext = nullptr,
			.buffer = res.buf
		};
		res.buffer_device_address = vkGetBufferDeviceAddress(current_device, &bda);
		switch(ret)
		{
			case VK_SUCCESS: break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				UNERR(tz::error_code::oom, "oom while creating buffer \"{}\"", name);
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				UNERR(tz::error_code::voom, "voom while creating buffer \"{}\"", name);
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				if(info.data.size_bytes() == 0)
				{
					UNERR(tz::error_code::precondition_failure, "zero-size buffers ({}) are not allowed", name);
				}
				else
				{
					UNERR(tz::error_code::driver_hazard, "unexpected failure when creating buffer \"{}\" due to an implementation-specific reason", name);
				}
			break;
			default:
				UNERR(tz::error_code::unknown_error, "undocumented vulkan error code returned when creating buffer \"{}\"", name);
			break;
		}

		res.data.resize(info.data.size());
		std::copy(info.data.begin(), info.data.end(), res.data.begin());

		return static_cast<tz::hanval>(hanval);
	}

	std::expected<resource_handle, tz::error_code> create_image(image_info info)
	{
		auto hanval = resources.size();
		resource_info& res = resources.emplace_back();
		res.res = info;
		std::string_view name = info.debug_name;
		if(name.empty())
		{
			name = "unnamed image";
		}

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
			case VK_SUCCESS: break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				UNERR(tz::error_code::oom, "oom while creating image \"{}\"", name);
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				UNERR(tz::error_code::voom, "voom while creating image \"{}\"", name);
			break;
			default:
				UNERR(tz::error_code::unknown_error, "undocumented vulkan error code returned when creating image \"{}\"", name);
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
			RETERR(tz::error_code::precondition_failure, "invalid resource handle - either the handle is garbage or you have already destroyed it.");
		}
		else if(info.is_buffer())
		{
			vmaDestroyBuffer(alloc, info.buf, info.mem);
			info = {};
		}
		else if(info.is_image())
		{
			vmaDestroyImage(alloc, info.img, info.mem);
			info = {};
		}
		return tz::error_code::success;
	}

	std::expected<shader_handle, tz::error_code> create_graphics_shader(std::string_view vertex_source, std::string_view fragment_source)
	{
		shaders.reserve(shaders.size() + 2);
		auto& vshad = shaders.emplace_back();
		std::uint16_t vid = shaders.size();
		vshad.ty = shader_type::vertex;
		auto& fshad = shaders.emplace_back();
		std::uint16_t fid = shaders.size();
		fshad.ty = shader_type::fragment;

		VkShaderModuleCreateInfo vcreate
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = vertex_source.size(),
			.pCode = reinterpret_cast<const std::uint32_t*>(vertex_source.data())
		};
		VkResult res = vkCreateShaderModule(current_device, &vcreate, nullptr, &vshad.smod);
		switch(res)
		{
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				UNERR(tz::error_code::voom, "ran out of GPU memory while trying to create vertex shader");
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				UNERR(tz::error_code::oom, "ran out of CPU memory while trying to create a vertex shader");
			break;
			case VK_SUCCESS: break;
			default:
				UNERR(tz::error_code::unknown_error, "undocumented vulkan error code reported when attempting to create vertex shader")
			break;
		}
		
		VkShaderModuleCreateInfo fcreate
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = fragment_source.size(),
			.pCode = reinterpret_cast<const std::uint32_t*>(fragment_source.data())
		};
		res = vkCreateShaderModule(current_device, &fcreate, nullptr, &fshad.smod);
		switch(res)
		{
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				UNERR(tz::error_code::voom, "ran out of GPU memory while trying to create fragment shader");
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				UNERR(tz::error_code::oom, "ran out of CPU memory while trying to create a fragment shader");
			break;
			case VK_SUCCESS: break;
			default:
				UNERR(tz::error_code::unknown_error, "undocumented vulkan error code reported when attempting to create fragment shader")
			break;
		}

		return static_cast<tz::hanval>((static_cast<std::uint32_t>(fid) << 16) + static_cast<std::uint32_t>(vid));
	}

	std::expected<shader_handle, tz::error_code> create_compute_shader(std::string_view compute_source)
	{
		auto& shad = shaders.emplace_back();
		std::size_t cid = shaders.size();
		shad.ty = shader_type::compute;

		VkShaderModuleCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = compute_source.size(),
			.pCode = reinterpret_cast<const std::uint32_t*>(compute_source.data())
		};
		VkResult res = vkCreateShaderModule(current_device, &create, nullptr, &shad.smod);
		switch(res)
		{
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				UNERR(tz::error_code::voom, "ran out of GPU memory while trying to create compute shader");
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				UNERR(tz::error_code::oom, "ran out of CPU memory while trying to create a compute shader");
			break;
			case VK_SUCCESS: break;
			default:
				UNERR(tz::error_code::unknown_error, "undocumented vulkan error code reported when attempting to create compute shader")
			break;
		}
		return static_cast<tz::hanval>(static_cast<std::uint32_t>(cid) << 16);
	}

	void destroy_shader(shader_handle handle)
	{
		auto top_part = (handle.peek() >> 16) & 0xFFFFFFFF;
		auto bottom_part = handle.peek() & 0x0000FFFF;
		if(bottom_part > 0x0)
		{
			auto& bottom_shader = shaders[--bottom_part];
			vkDestroyShaderModule(current_device, bottom_shader.smod, nullptr);
			bottom_shader = {};
		}
		auto& top_shader = shaders[--top_part];
		vkDestroyShaderModule(current_device, top_shader.smod, nullptr);
		top_shader = {};
	}

	std::expected<pass_handle, tz::error_code> create_pass(pass_info info)
	{
		if(info.shader == tz::nullhand)
		{
			UNERR(tz::error_code::precondition_failure, "no shader program provided when creating pass. you must provide a valid shader program.");
		}
		std::size_t ret_id = passes.size();
		auto top_part = (info.shader.peek() >> 16) & 0xFFFFFFFF;
		auto& shader1 = shaders[--top_part];
		auto bottom_part = info.shader.peek() & 0x0000FFFF;

		auto& pass = passes.emplace_back();
		VkResult res = VK_SUCCESS;

		if(shader1.ty == shader_type::compute)
		{
			VkComputePipelineCreateInfo create
			{
				.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = VkPipelineShaderStageCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_COMPUTE_BIT,
					.module = shader1.smod,
					.pName = "main",
					.pSpecializationInfo = nullptr
				},
				.layout = default_layout,
				.basePipelineHandle = VK_NULL_HANDLE,
				.basePipelineIndex = -1
			};
			// create compute pipeline and record etc...
			res = vkCreateComputePipelines(current_device, VK_NULL_HANDLE, 1, &create, nullptr, &pass.pipeline);
		}
		else
		{
			if(bottom_part == 0x0)
			{
				UNERR(tz::error_code::precondition_failure, "provided a shader program consisting of only 1 shader, and that shader is not a compute shader. a graphics shader program must be comprised of both a vertex shader and fragment shader");
			}
			auto& shader2 = shaders[--bottom_part];

			std::array<VkPipelineShaderStageCreateInfo, 2> shader_creates
			{
				VkPipelineShaderStageCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_VERTEX_BIT,
					.module = shader2.smod,
					.pName = "main",
					.pSpecializationInfo = nullptr
				},
				VkPipelineShaderStageCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.module = shader1.smod,
					.pName = "main",
					.pSpecializationInfo = nullptr
				}
			};

			if(info.graphics.colour_targets.empty())
			{
				UNERR(tz::error_code::precondition_failure, "detected graphics pass with no colour targets. a graphics pass must have *at least* one colour target");
			}
			resource_handle first_colour_target = info.graphics.colour_targets.front();
			std::uint32_t viewport_width = 0;
			std::uint32_t viewport_height = 0;
			if(first_colour_target == tz::nullhand)
			{
				UNERR(tz::error_code::precondition_failure, "first colour target passed into graphics pass info {} is the null resource. the list of colour targets must not contain a null resource", info.debug_name);
			}
			if(first_colour_target == window_resource)
			{
				// drawing into the window!
				tz::error_code err = impl_need_swapchain(tz::os::window_get_width(), tz::os::window_get_height());
				if(err == tz::error_code::success || err == tz::error_code::partial_success)
				{
					viewport_width = swapchain_width;
					viewport_height = swapchain_height;
				}
				else
				{
					UNERR(err, "error while retrieving window as target: {}", tz::last_error());
				}
			}
			else
			{
				// first_colour_target is an actual resource. it better be an image.
				const auto& colour_target_res = resources[first_colour_target.peek()];
				if(colour_target_res.is_invalid())
				{
					UNERR(tz::error_code::precondition_failure, "first colour target passed into graphics pass info {} is an invalid resource", info.debug_name);
				}
				else if(colour_target_res.is_buffer())
				{
					UNERR(tz::error_code::precondition_failure, "first colour target passed into graphics pass info {} is buffer resource \"{}\" -- it must be an image resource", info.debug_name, std::get<buffer_info>(colour_target_res.res).debug_name);
				}
				else if(colour_target_res.is_image())
				{
					const auto& img = std::get<image_info>(colour_target_res.res);
					viewport_width = img.width;
					viewport_height = img.height;
				}
				else
				{
					UNERR(tz::error_code::engine_bug, "first colour target passed into graphicspass info {} is corrupt: neither \"invalid\", \"buffer\" nor \"resource\", which should be impossible. likely memory corruption.", info.debug_name);
				}
			}

			VkPipelineVertexInputStateCreateInfo vtx
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.vertexBindingDescriptionCount = 0,
				.pVertexBindingDescriptions = nullptr,
				.vertexAttributeDescriptionCount = 0,
				.pVertexAttributeDescriptions = nullptr
			};

			VkPipelineInputAssemblyStateCreateInfo iasm
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE
			};

			VkPipelineTessellationStateCreateInfo tess
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.patchControlPoints = 3
			};

			// swapchain width/height are not safe to use here, and are wrong for passes that render into a texture.

			VkViewport vp
			{
				.x = 0.0f,
				.y = static_cast<float>(viewport_height),
				.width = static_cast<float>(viewport_width),
				.height = -static_cast<float>(viewport_height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			VkRect2D sci
			{
				.offset = {0, 0},
				.extent =
				{
					.width = viewport_width,
					.height = viewport_height
				}
			};

			VkPipelineViewportStateCreateInfo viewport
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.viewportCount = 1,
				.pViewports = &vp,
				.scissorCount = 1,
				.pScissors = &sci
			};

			VkPipelineRasterizationStateCreateInfo raster
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.depthClampEnable = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode = VK_POLYGON_MODE_FILL,
				.cullMode = VK_CULL_MODE_BACK_BIT,
				.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
				.depthBiasEnable = VK_FALSE,
				.depthBiasConstantFactor = 0.0f,
				.depthBiasClamp = 0.0f,
				.depthBiasSlopeFactor = 0.0f,
				.lineWidth = 1.0f
			};

			VkPipelineMultisampleStateCreateInfo ms
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE,
				.minSampleShading = 1.0f,
				.pSampleMask = nullptr,
				.alphaToCoverageEnable = VK_FALSE,
				.alphaToOneEnable = VK_FALSE
			};
			
			VkPipelineDepthStencilStateCreateInfo depth
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.depthTestEnable = VK_TRUE,
				.depthWriteEnable = VK_TRUE,
				.depthCompareOp = VK_COMPARE_OP_LESS,
				.depthBoundsTestEnable = VK_TRUE,
				.stencilTestEnable = VK_FALSE,
				.front = VkStencilOpState{},
				.back = VkStencilOpState{},
				.minDepthBounds = 0.0f,
				.maxDepthBounds = 1.0f	
			};

			VkPipelineColorBlendAttachmentState no_blending
			{
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};

			VkPipelineColorBlendStateCreateInfo blend
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.logicOpEnable = VK_FALSE,
				.logicOp = VkLogicOp{},
				.attachmentCount = 1,
				.pAttachments = &no_blending,
				.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
			};

			VkFormat col_form = impl_get_format_from_image_type(tz::gpu::image_type::rgba);

			// dynamic rendering requires extra:
			VkPipelineRenderingCreateInfo rendering
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = nullptr,
				.viewMask = 0,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &col_form,
				.depthAttachmentFormat = impl_get_format_from_image_type(tz::gpu::image_type::depth),
				.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
			};
			
			VkGraphicsPipelineCreateInfo create
			{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = &rendering,
				.flags = 0,
				.stageCount = 2,
				.pStages = shader_creates.data(),
				.pVertexInputState = &vtx,
				.pInputAssemblyState = &iasm,
				.pTessellationState = &tess,
				.pViewportState = &viewport,
				.pRasterizationState = &raster,
				.pMultisampleState = &ms,
				.pDepthStencilState = &depth,
				.pColorBlendState = &blend,
				.pDynamicState = nullptr, // todo
				.layout = default_layout,
				.renderPass = VK_NULL_HANDLE,
				.subpass = 0,
				.basePipelineHandle = VK_NULL_HANDLE,
				.basePipelineIndex = -1
			};
			// create graphics pipeline and record etc...
			res = vkCreateGraphicsPipelines(current_device, VK_NULL_HANDLE, 1, &create, nullptr, &pass.pipeline);
		}

		switch(res)
		{
			case VK_SUCCESS: break;
			default:
				UNERR(tz::error_code::unknown_error, "failed to create vulkan pipeline");
			break;
		}

		pass.info = info;
		pass.layout = default_layout;
		pass_handle ret = static_cast<tz::hanval>(ret_id);
		impl_write_all_resources(ret);
		impl_record_gpu_work(ret);
		return ret;
	}

	void destroy_pass(pass_handle pass)
	{
		auto i = pass.peek();
		tz_assert(passes.size() > i, "Dodgy handle (value {}) passed to destroy_pass", i);
		vkDestroyPipeline(current_device, passes[i].pipeline, nullptr);
		passes[i] = {};
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
		if(!features12.timelineSemaphore)
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
					.flags = 0,
					.hinstance = GetModuleHandle(nullptr),
					.hwnd = reinterpret_cast<HWND>(static_cast<std::uintptr_t>(wnd.peek()))
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
		auto pdev = reinterpret_cast<VkPhysicalDevice>(static_cast<std::uintptr_t>(current_hardware.internals.i0.peek()));
		VkSurfaceCapabilitiesKHR surface_caps;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdev, surface, &surface_caps);
		// recreate swapchain.
		w = std::clamp(w, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width);
		h = std::clamp(h, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
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
			.preTransform = surface_caps.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
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
				return VK_FORMAT_D32_SFLOAT;
			break;
			case tz::gpu::image_type::floats:
				return VK_FORMAT_R32_SFLOAT;
			break;
			default:
				return VK_FORMAT_UNDEFINED;
			break;
		}
	}

	void impl_write_all_resources(pass_handle passh)
	{
		VkCommandBufferBeginInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		// todo: error checking.
		vkBeginCommandBuffer(scratch_cmds, &create);
		const auto& pass = passes[passh.peek()];
		if(pass.info.resources.empty())
		{
			return;
		}
		for(const auto& resh : pass.info.resources)
		{
			const auto& res = resources[resh.peek()];	
			if(res.is_buffer())
			{
				const auto& buffer = std::get<buffer_info>(res.res);
				(void)buffer;
			}
			else if(res.is_image())
			{
				const auto& image = std::get<image_info>(res.res);
				(void)image;
			}
			else
			{
				tz_error("ruh roh");
			}
		}

		vkEndCommandBuffer(scratch_cmds);
		VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_NONE;
		VkSubmitInfo submit
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = &stage_mask,
			.commandBufferCount = 1,
			.pCommandBuffers = &scratch_cmds,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = nullptr
		};
		// todo: check errors.
		vkQueueSubmit(graphics_compute_queue, 1, &submit, scratch_fence);	
		vkWaitForFences(current_device, 1, &scratch_fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	}

	void impl_record_compute_work(const pass_data& pass, const frame_data_t& frame);
	void impl_record_graphics_work(const pass_data& pass, const frame_data_t& frame);

	void impl_record_gpu_work(pass_handle passh)
	{
		const auto& pass = passes[passh.peek()]; 
		for(std::size_t i = 0; i < frame_overlap; i++)
		{
			const auto& frame = frames[i];
			vkResetCommandBuffer(frame.cmds, 0);
			VkCommandBufferBeginInfo begin
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = 0,
				.pInheritanceInfo = nullptr
			};
			vkBeginCommandBuffer(frame.cmds, &begin);
			// GPU work goes here.
			auto top_part = (pass.info.shader.peek() >> 16) & 0xFFFFFFFF;
			auto& shader1 = shaders[--top_part];
			if(shader1.ty == shader_type::compute)
			{
				impl_record_compute_work(pass, frame);
			}
			else
			{
				impl_record_graphics_work(pass, frame);
			}
			vkEndCommandBuffer(frame.cmds);
		}
	}

	void impl_record_compute_work(const pass_data& pass, const frame_data_t& frame)
	{
		(void)pass;
		(void)frame;
	}

	void impl_record_graphics_work(const pass_data& pass, const frame_data_t& frame)
	{
		(void)pass;
		(void)frame;
	}


	VkPipelineLayout impl_create_layout()
	{
		VkDescriptorSetLayoutBinding images_binding
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = max_image_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		};
		VkDescriptorBindingFlags flags =
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = 1,
			.pBindingFlags = &flags
		};
		VkDescriptorSetLayoutCreateInfo dlcreate
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &flags_create,
			.flags =
				VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = 1,
			.pBindings = &images_binding
		};
		for(std::size_t i = 0; i < frame_overlap; i++)
		{
			vkCreateDescriptorSetLayout(current_device, &dlcreate, nullptr, &set_layouts[i]);
		}
		VkPipelineLayoutCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = frame_overlap,
			.pSetLayouts = set_layouts.data(),
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
		VkPipelineLayout ret;
		VkResult res = vkCreatePipelineLayout(current_device, &create, nullptr, &ret);
		switch(res)
		{
			default: break;
		}
		return ret;
	}
}
#endif