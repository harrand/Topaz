#if TOPAZ_VULKAN
#include "tz/topaz.hpp"
#include "tz/gpu/hardware.hpp"
#include "tz/gpu/resource.hpp"
#include "tz/gpu/shader.hpp"
#include "tz/gpu/pass.hpp"
#include "tz/gpu/graph.hpp"
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
	std::vector<VkImageView> swapchain_views = {};
	hardware current_hardware;
	#define VULKAN_API_VERSION_USED VK_API_VERSION_1_3
	constexpr VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
	VmaAllocator alloc = VK_NULL_HANDLE;
	VkQueue graphics_compute_queue = VK_NULL_HANDLE;
	VkPipelineLayout default_layout = VK_NULL_HANDLE;
	constexpr std::uint32_t max_global_image_count = 8192;
	constexpr std::uint32_t max_image_count_per_pass = 1024;

	VkImage system_image = VK_NULL_HANDLE;
	VmaAllocation system_image_mem = VK_NULL_HANDLE;
	VkImageView system_image_view = VK_NULL_HANDLE;
	VkImage system_depth_image = VK_NULL_HANDLE;
	VmaAllocation system_depth_image_mem = VK_NULL_HANDLE;
	VkImageView system_depth_image_view = VK_NULL_HANDLE;

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
	std::size_t current_frame = 0;
	std::array<VkDescriptorSetLayout, frame_overlap> set_layouts = {};
	std::vector<VkDescriptorPool> descriptor_pools = {};
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
		VkImageView img_view = VK_NULL_HANDLE;
		VkSampler img_sampler = VK_NULL_HANDLE;
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
	struct scratchbuf_t
	{
		VkBuffer buf;
		VmaAllocation mem;
	};
	std::vector<scratchbuf_t> scratch_buffers = {};
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
		std::array<VkDescriptorSet, frame_overlap> descriptor_sets = {};
		std::uint32_t viewport_width = 0;
		std::uint32_t viewport_height = 0;
	};
	std::vector<pass_data> passes = {};

	struct graph_data
	{
		std::vector<pass_handle> timeline = {};
		std::vector<std::vector<pass_handle>> dependencies = {};
	};
	std::vector<graph_data> graphs = {};

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
	void impl_populate_descriptors(pass_handle passh);
	VkPipelineLayout impl_create_layout();
	// call this when you have need of the swapchain.
	// returns success if a swapchain already exists and is ready for use.
	// returns partial success if a new swapchain has been created (typically if this is your first time calling it, or if the old swapchain was out-of-date). you might want to rerecord commands.
	// returns partial_success if the swapchain previously existed, but has been recreated for some important reason (you will maybe need to rerecord commands)
	// returns oom if oom, voom if voom
	// returns unknown_error if some undocumented vulkan error occurred.
	std::pair<std::uint32_t, std::uint32_t> impl_get_image_dimensions(tz::gpu::resource_handle imagey_resource);
	tz::error_code impl_need_swapchain(std::uint32_t w, std::uint32_t h);
	VkFormat impl_get_format_from_image_type(tz::gpu::image_type type);
	tz::error_code impl_validate_colour_targets(tz::gpu::pass_info& pinfo, pass_data& data);
	tz::error_code impl_cmd_resource_write(VkCommandBuffer cmds, resource_handle resource, std::span<const std::byte> newdata, std::size_t offset = 0);
	void impl_write_all_resources(pass_handle pass);
	tz::error_code impl_record_gpu_work(pass_handle pass, std::size_t i);
	void impl_pass_go(pass_handle pass);
	void impl_destroy_system_images();

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
		if(current_device != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(current_device);
		}

		for(std::size_t i = 0 ; i < resources.size(); i++)
		{
			if(!resources[i].is_invalid())
			{
				tz_must(destroy_resource(static_cast<tz::hanval>(i)));
			}
		}
		for(const auto& scratch : scratch_buffers)
		{
			vmaDestroyBuffer(alloc, scratch.buf, scratch.mem);
		}
		scratch_buffers.clear();

		if(swapchain != VK_NULL_HANDLE)
		{
			for(const VkImageView view : swapchain_views)
			{
				vkDestroyImageView(current_device, view, nullptr);
			}
			swapchain_views.clear();
			vkDestroySwapchainKHR(current_device, swapchain, nullptr);
			swapchain = VK_NULL_HANDLE;
			impl_destroy_system_images();
		}
		if(alloc != VK_NULL_HANDLE)
		{
			vmaDestroyAllocator(alloc);
			alloc = VK_NULL_HANDLE;
		}
		if(current_device != VK_NULL_HANDLE)
		{
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
			for(const auto dpool : descriptor_pools)
			{
				vkDestroyDescriptorPool(current_device, dpool, nullptr);
			}
			descriptor_pools.clear();
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
		std::string_view name = info.name;
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
				// add transfer_dst because static resources will need a transfer command to set their data.
				create.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
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
		std::string_view name = info.name;
		if(name.empty())
		{
			name = "unnamed image";
		}
		VkFormat fmt = impl_get_format_from_image_type(info.type);

		VkImageCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = fmt,
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
				create.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			break;
			case tz::gpu::resource_access::dynamic_access:
				alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
				alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			break;
		}
		if(info.flags & image_flag::colour_target)
		{
			create.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if(info.flags & image_flag::depth_target)
		{
			create.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}

		VmaAllocationInfo alloc_result;
		VkResult ret = vmaCreateImage(alloc, &create, &alloc_info, &res.img, &res.mem, &alloc_result);
		if(info.access == tz::gpu::resource_access::dynamic_access)
		{
			res.buffer_mapped_address = alloc_result.pMappedData;
		}
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

		VkImageViewCreateInfo view_create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = res.img,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = fmt,
			.components = VkComponentMapping{},
			.subresourceRange =
			{
				.aspectMask = static_cast<VkImageAspectFlags>(info.type == tz::gpu::image_type::depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vkCreateImageView(current_device, &view_create, nullptr, &res.img_view);
		VkSamplerCreateInfo samp_create
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_NEAREST,
			.minFilter = VK_FILTER_NEAREST,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 0.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE
		};
		vkCreateSampler(current_device, &samp_create, nullptr, &res.img_sampler);

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
			vkDestroyImageView(current_device, info.img_view, nullptr);
			vkDestroySampler(current_device, info.img_sampler, nullptr);
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
			tz::error_code colour_target_validity = impl_validate_colour_targets(info, pass);
			if(colour_target_validity != tz::error_code::success)
			{
				// already sets error message, so just propagate the error code.
				return std::unexpected(colour_target_validity);
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

			VkPipelineViewportStateCreateInfo viewport
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.viewportCount = 1,
				.pViewports = nullptr,
				.scissorCount = 1,
				.pScissors = nullptr
			};

			VkCullModeFlags cull_bits;
			switch(info.graphics.culling)
			{
				case cull::both:
					cull_bits = VK_CULL_MODE_FRONT_AND_BACK;
				break;
				case cull::back:
					cull_bits = VK_CULL_MODE_BACK_BIT;
				break;
				case cull::front:
					cull_bits = VK_CULL_MODE_FRONT_BIT;
				break;
				case cull::none:
					cull_bits = VK_CULL_MODE_NONE;
				break;
			}

			VkPipelineRasterizationStateCreateInfo raster
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.depthClampEnable = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode = VK_POLYGON_MODE_FILL,
				.cullMode = cull_bits,
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
				.depthTestEnable = (info.graphics.flags & graphics_flag::no_depth_test) ? VK_FALSE : VK_TRUE,
				.depthWriteEnable = (info.graphics.flags & graphics_flag::no_depth_write) ? VK_FALSE : VK_TRUE,
				.depthCompareOp = VK_COMPARE_OP_LESS,
				.depthBoundsTestEnable = VK_TRUE,
				.stencilTestEnable = VK_FALSE,
				.front = VkStencilOpState{},
				.back = VkStencilOpState{},
				.minDepthBounds = 0.0f,
				.maxDepthBounds = 1.0f	
			};

			std::vector<VkPipelineColorBlendAttachmentState> blend_states;
			blend_states.resize(info.graphics.colour_targets.size(),
				// no blending.
				VkPipelineColorBlendAttachmentState
				{
					.blendEnable = VK_FALSE,
					.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
					.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
					.colorBlendOp = VK_BLEND_OP_ADD,
					.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
					.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
					.alphaBlendOp = VK_BLEND_OP_ADD,
					.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
				}
			);

			VkPipelineColorBlendStateCreateInfo blend
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.logicOpEnable = VK_FALSE,
				.logicOp = VkLogicOp{},
				.attachmentCount = static_cast<std::uint32_t>(blend_states.size()),
				.pAttachments = blend_states.data(),
				.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
			};

			std::vector<VkFormat> col_forms;
			col_forms.resize(info.graphics.colour_targets.size(), impl_get_format_from_image_type(tz::gpu::image_type::rgba));

			// dynamic rendering requires extra:
			VkPipelineRenderingCreateInfo rendering
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.pNext = nullptr,
				.viewMask = 0,
				.colorAttachmentCount = static_cast<std::uint32_t>(col_forms.size()),
				.pColorAttachmentFormats = col_forms.data(),
				.depthAttachmentFormat = impl_get_format_from_image_type(tz::gpu::image_type::depth),
				.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
			};
			
			VkDynamicState dynamic_states[] =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};

			VkPipelineDynamicStateCreateInfo dynamic
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]),
				.pDynamicStates = dynamic_states
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
				.pDynamicState = &dynamic, // todo
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
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				UNERR(tz::error_code::oom, "ran out of CPU memory while trying to create vulkan pipeline");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				UNERR(tz::error_code::voom, "ran out of GPU memory while trying to create vulkan pipeline");
			break;
			default:
				UNERR(tz::error_code::unknown_error, "failed to create vulkan pipeline");
			break;
		}

		pass.info = info;
		pass.layout = default_layout;
		pass_handle ret = static_cast<tz::hanval>(ret_id);
		impl_write_all_resources(ret);
		impl_populate_descriptors(ret);
		return ret;
	}

	void destroy_pass(pass_handle pass)
	{
		auto i = pass.peek();
		tz_assert(passes.size() > i, "Dodgy handle (value {}) passed to destroy_pass", i);
		vkDestroyPipeline(current_device, passes[i].pipeline, nullptr);
		passes[i] = {};
	}

	std::expected<graph_handle, tz::error_code> create_graph(graph_info info)
	{
		if(info.dependencies.size() > info.timeline.size())
		{
			UNERR(tz::error_code::precondition_failure, "invalid graph - {} sets of dependencies but only {} passes in the timeline. the number of sets of dependencies should be less than or equal to the number of passes.", info.dependencies.size(), info.timeline.size());
		}
		std::size_t ret_id = graphs.size();
		auto& graph = graphs.emplace_back();
		// copy over timeline and dependencies
		graph.timeline.resize(info.timeline.size());
		std::copy(info.timeline.begin(), info.timeline.end(), graph.timeline.begin());
		
		graph.dependencies.resize(info.dependencies.size());
		for(std::size_t i = 0; i < info.dependencies.size(); i++)
		{
			const auto& deps = info.dependencies[i];
			for(pass_handle dep : deps)
			{
				graph.dependencies[i].push_back(dep);
			}
		}

		graph_handle ret = static_cast<tz::hanval>(ret_id);
		return ret;
	}

	void execute(graph_handle graphh)
	{
		const auto& frame = frames[current_frame];
		const auto& graph = graphs[graphh.peek()];

		std::uint32_t image_index;
		vkAcquireNextImageKHR(current_device, swapchain, std::numeric_limits<std::uint64_t>::max(), VK_NULL_HANDLE, frame.swapchain_fence, &image_index);
		vkWaitForFences(current_device, 1, &frame.swapchain_fence, VK_TRUE, std::numeric_limits<std::uint32_t>::max());
		vkResetFences(current_device, 1, &frame.swapchain_fence);

		std::uint64_t begin_point;
		// todo: ec
		// whats the value of our semaphore when the frame starts?
		// this is our new 0
		vkGetSemaphoreCounterValue(current_device, frame.timeline_sem, &begin_point);

		VkImageMemoryBarrier barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_GENERAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapchain_images[image_index],
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		// record all commands jit
		vkResetCommandBuffer(frame.cmds, 0);
		VkCommandBufferBeginInfo begin
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		// go go go
		vkBeginCommandBuffer(frame.cmds, &begin);
		// transition swapchain image to general
		vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		// go through timeline and record all gpu work.
		for(pass_handle pass : graph.timeline)
		{
			impl_record_gpu_work(pass, current_frame);
		}
		// transition swapchain image to present
		barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		vkEndCommandBuffer(frame.cmds);
		VkPipelineStageFlags stage_mask = VK_PIPELINE_STAGE_NONE;
		VkSubmitInfo submit
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = &stage_mask,
			.commandBufferCount = 1,
			.pCommandBuffers = &frame.cmds,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &frame.swapchain_sem
		};
		vkQueueSubmit(graphics_compute_queue, 1, &submit, frame.swapchain_fence);
		vkWaitForFences(current_device, 1, &frame.swapchain_fence, VK_TRUE, std::numeric_limits<std::uint32_t>::max());
		vkResetFences(current_device, 1, &frame.swapchain_fence);

		VkResult res;
		VkPresentInfoKHR present
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &frame.swapchain_sem,
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &image_index,
			.pResults = &res
		};
		vkQueuePresentKHR(graphics_compute_queue, &present);

		current_frame = (current_frame + 1) % frame_overlap;
	}

	void destroy_graph(graph_handle graph)
	{
		(void)graph;
		tz_error("graph destruction on vulkan is not yet implemented.");
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
				break;
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

	std::pair<std::uint32_t, std::uint32_t> impl_get_image_dimensions(tz::gpu::resource_handle colour_target)
	{
		if(colour_target == tz::nullhand)
		{
			return {0, 0};
		}
		if(colour_target == window_resource)
		{
			// drawing into the window!
			tz::error_code err = impl_need_swapchain(tz::os::window_get_width(), tz::os::window_get_height());
			if(err == tz::error_code::success || err == tz::error_code::partial_success)
			{
				return {swapchain_width, swapchain_height};
			}
		}
		else
		{
			// colour_target is an actual resource. it better be an image.
			const auto& colour_target_res = resources[colour_target.peek()];
			if(colour_target_res.is_image())
			{
				const auto& img = std::get<image_info>(colour_target_res.res);
				return {img.width, img.height};
			}
		}
		return {0, 0};
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
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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
		for(VkImageView view : swapchain_views)
		{
			vkDestroyImageView(current_device, view, nullptr);
		}
		swapchain_views.clear();
		std::uint32_t swapchain_image_count;
		vkGetSwapchainImagesKHR(current_device, swapchain, &swapchain_image_count, nullptr);
		swapchain_images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(current_device, swapchain, &swapchain_image_count, swapchain_images.data());

		VkImageViewCreateInfo view_create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = VK_NULL_HANDLE, // note for code below this: set this when you create a copy for use
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchain_format,
			.components = VkComponentMapping{},
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		for(VkImage swapchain_img : swapchain_images)
		{
			// make copy and point to swapchain image.
			view_create.image = swapchain_img;
			VkImageView& view = swapchain_views.emplace_back();
			vkCreateImageView(current_device, &view_create, nullptr, &view);
		}

		// create system image and depth image (an internal image that is rendered into when the user wants to render "into the window". it is blitted to a swapchain image, saves us having to worry about the swapchain image being in a different format.)
		// let's delete the old ones first if they exist...
		impl_destroy_system_images();

		VkImageCreateInfo system_image_create
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = impl_get_format_from_image_type(tz::gpu::image_type::rgba),
			.extent =
			{
				.width = w,
				.height = h,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // can be rendered into but also transfer source (system image -> swapchain image blit)
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &current_hardware.internals.i1,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		VmaAllocationCreateInfo system_image_alloc =
		{
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST
		};
		vmaCreateImage(alloc, &system_image_create, &system_image_alloc, &system_image, &system_image_mem, nullptr);

		view_create.image = system_image;
		view_create.format = system_image_create.format;
		vkCreateImageView(current_device, &view_create, nullptr, &system_image_view);

		// depth image.
		system_image_create.format = impl_get_format_from_image_type(tz::gpu::image_type::depth);
		system_image_create.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		vmaCreateImage(alloc, &system_image_create, &system_image_alloc, &system_depth_image, &system_depth_image_mem, nullptr);

		view_create.image = system_depth_image;
		view_create.format = system_image_create.format;
		view_create.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		vkCreateImageView(current_device, &view_create, nullptr, &system_depth_image_view);

		return tz::error_code::partial_success;
	}

	tz::error_code impl_cmd_resource_write(VkCommandBuffer cmds, resource_handle resource, std::span<const std::byte> newdata, std::size_t offset)
	{
		auto& res = resources[resource.peek()];
		if(res.is_buffer())
		{
			const auto& buffer = std::get<buffer_info>(res.res);
			if(buffer.data.size_bytes() < (newdata.size_bytes() + offset))
			{
				RETERR(tz::error_code::precondition_failure, "attempt to write {} bytes into buffer {} at offset {}, which is only {} bytes large", newdata.size_bytes(), buffer.name, offset, buffer.data.size_bytes());
			}

			if(buffer.access == tz::gpu::resource_access::dynamic_access)
			{
				void* addr = res.buffer_mapped_address;
				addr = ((char*)addr) + offset;
				std::memcpy(addr, newdata.data(), newdata.size_bytes());
				return tz::error_code::success;
			}

			scratchbuf_t& scratchbuf = scratch_buffers.emplace_back();
			VkBufferCreateInfo create
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = newdata.size_bytes(),
				.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 1,
				.pQueueFamilyIndices = &current_hardware.internals.i1
			};
			VmaAllocationCreateInfo alloc_create
			{
				.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			};
			VmaAllocationInfo alloc_out;
			vmaCreateBuffer(alloc, &create, &alloc_create, &scratchbuf.buf, &scratchbuf.mem, &alloc_out);
			std::memcpy(alloc_out.pMappedData, newdata.data(), newdata.size_bytes());
			VkBufferCopy cpy
			{
				.srcOffset = 0,
				.dstOffset = offset,
				.size = newdata.size_bytes()
			};
			vkCmdCopyBuffer(cmds, scratchbuf.buf, res.buf, 1, &cpy);
			// done. scratch buffers will be cleared up later.
		}
		else if(res.is_image())
		{
			const auto& image = std::get<image_info>(res.res);
			if(image.data.size_bytes() < (newdata.size_bytes() + offset))
			{
				RETERR(tz::error_code::precondition_failure, "attempt to write {} bytes into image {} at offset {}, which is only {} bytes large", newdata.size_bytes(), image.name, offset, image.data.size_bytes());
			}

			if(image.access == tz::gpu::resource_access::dynamic_access)
			{
				void* addr = res.buffer_mapped_address;
				addr = ((char*)addr) + offset;
				std::memcpy(addr, newdata.data(), newdata.size_bytes());
				return tz::error_code::success;
			}

			scratchbuf_t& scratchbuf = scratch_buffers.emplace_back();
			VkBufferCreateInfo create
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = newdata.size_bytes(),
				.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 1,
				.pQueueFamilyIndices = &current_hardware.internals.i1
			};
			VmaAllocationCreateInfo alloc_create
			{
				.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
			};
			VmaAllocationInfo alloc_out;
			vmaCreateBuffer(alloc, &create, &alloc_create, &scratchbuf.buf, &scratchbuf.mem, &alloc_out);
			std::memcpy(alloc_out.pMappedData, newdata.data(), newdata.size_bytes());
			VkBufferImageCopy cpy
			{
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = VkImageSubresourceLayers
				{
					.aspectMask = static_cast<VkImageAspectFlags>(image.type == tz::gpu::image_type::depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			VkImageMemoryBarrier barrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_GENERAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = res.img,
				.subresourceRange = VkImageSubresourceRange
				{
					.aspectMask = static_cast<VkImageAspectFlags>(image.type == tz::gpu::image_type::depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};
			vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			vkCmdCopyBufferToImage(cmds, scratchbuf.buf, res.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
			// done. scratch buffers will be cleared up later.
		}
		return tz::error_code::success;
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
				impl_cmd_resource_write(scratch_cmds, resh, buffer.data, 0);
			}
			else if(res.is_image())
			{
				const auto& image = std::get<image_info>(res.res);
				impl_cmd_resource_write(scratch_cmds, resh, image.data, 0);
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

	tz::error_code impl_record_compute_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id);
	tz::error_code impl_record_graphics_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id);

	tz::error_code impl_record_gpu_work(pass_handle passh, std::size_t i)
	{
		const auto& pass = passes[passh.peek()]; 
		tz::error_code ret = tz::error_code::success;
		const auto& frame = frames[i];
		// GPU work goes here.
		auto top_part = (pass.info.shader.peek() >> 16) & 0xFFFFFFFF;
		auto& shader1 = shaders[--top_part];
		if(shader1.ty == shader_type::compute)
		{
			ret = impl_record_compute_work(pass, frame, i);
		}
		else
		{
			ret = impl_record_graphics_work(pass, frame, i);
		}
		return ret;
	}

	tz::error_code impl_record_compute_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id)
	{
		vkCmdBindPipeline(frame.cmds, VK_PIPELINE_BIND_POINT_COMPUTE, pass.pipeline);
		vkCmdBindDescriptorSets(frame.cmds, VK_PIPELINE_BIND_POINT_COMPUTE, pass.layout, 0u, 1, pass.descriptor_sets.data() + id, 0, nullptr);
		// dispatch

		return tz::error_code::success;
	}

	tz::error_code impl_record_graphics_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id)
	{
		std::vector<VkRenderingAttachmentInfo> colour_attachments;
		std::vector<VkImageMemoryBarrier> colour_transitions;
		colour_attachments.reserve(pass.info.graphics.colour_targets.size());
		colour_transitions.reserve(pass.info.graphics.colour_targets.size());
		bool render_into_system_image = false;
		for(const auto colour_resh : pass.info.graphics.colour_targets)
		{
			VkImage rt = VK_NULL_HANDLE;
			VkImageView rtv = VK_NULL_HANDLE;
			if(colour_resh == window_resource)
			{
				rt = system_image;
				rtv = system_image_view;
				render_into_system_image = true;
			}
			else
			{
				const auto& res = resources[colour_resh.peek()];
				rt = res.img;
				rtv = res.img_view;
			}
			colour_attachments.push_back
			({
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext = nullptr,
				.imageView = rtv,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.resolveImageView = VK_NULL_HANDLE,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp = (pass.info.graphics.flags & graphics_flag::dont_clear) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR, // clear colour target before rendered into
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			});
			if(!(pass.info.graphics.flags & graphics_flag::dont_clear))
			{
				colour_transitions.push_back({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0,
					.dstAccessMask = 0,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.newLayout = VK_IMAGE_LAYOUT_GENERAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = rt,
					.subresourceRange = VkImageSubresourceRange
					{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1,
					}
				});
			}
		}
		if(colour_transitions.size())
		{
			vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, colour_transitions.size(), colour_transitions.data());
		}
		VkImageView depth_rtv = VK_NULL_HANDLE;
		if(pass.info.graphics.depth_target != tz::nullhand)
		{
			if(pass.info.graphics.depth_target == tz::gpu::window_resource)
			{
				depth_rtv = system_depth_image_view;
			}
			else
			{
				const auto& depth_image_resource = resources[pass.info.graphics.depth_target.peek()];
				if(depth_image_resource.is_invalid())
				{
					RETERR(tz::error_code::precondition_failure, "invalid resource handle provided as depth target to pass \"{}\"", pass.info.name);
				}
				else if(depth_image_resource.is_buffer())
				{
					RETERR(tz::error_code::precondition_failure, "resource provided as depth target to pass \"{}\" was not an image, but infact a buffer", pass.info.name);
				}
				tz_assert(depth_image_resource.is_image(), "non-image passed a depth target");
				const auto& depth_res = std::get<tz::gpu::image_info>(depth_image_resource.res);
				if(!(depth_res.flags & tz::gpu::image_flag::depth_target))
				{
					RETERR(tz::error_code::precondition_failure, "image resource \"{}\" provided as depth target to pass \"{}\", but the image does not have the \"depth_target\" flag.", depth_res.name, pass.info.name);
				}
				depth_rtv = depth_image_resource.img_view;
			}
		}
		else
		{
			if(!(pass.info.graphics.flags & graphics_flag::no_depth_test))
			{
				RETERR(tz::error_code::precondition_failure, "no depth target was provided for pass \"{}\". you either need to pass \"graphics_flag::no_depth_test\" to disable depth testing entirely, or pass a valid depth target.", pass.info.name);
			}
		}
		VkRenderingAttachmentInfo maybe_depth
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = nullptr,
			.imageView = depth_rtv,
			.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = (pass.info.graphics.flags & graphics_flag::dont_clear) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // potentially want to store if another pass is going to read from depth afterwards (in the timeline)
		};
		VkRenderingInfo render
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderArea =
			{
				.offset = {0, 0},
				.extent = {pass.viewport_width, pass.viewport_height}
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = static_cast<std::uint32_t>(colour_attachments.size()),
			.pColorAttachments = colour_attachments.data(),
			.pDepthAttachment = (pass.info.graphics.flags & graphics_flag::no_depth_test) ? nullptr : &maybe_depth,
			.pStencilAttachment = nullptr
		};
		// first: transition swapchain image layout from undefined (trashes data) to colour attachment (if we're rendering directly into the window and we're the first pass in the timeline) OR general
		vkCmdBeginRendering(frame.cmds, &render);
		vkCmdBindPipeline(frame.cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipeline);
		vkCmdBindDescriptorSets(frame.cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.layout, 0u, 1, pass.descriptor_sets.data() + id, 0, nullptr);

		VkViewport vp
		{
			.x = 0.0f,
			.y = static_cast<float>(pass.viewport_height),
			.width = static_cast<float>(pass.viewport_width),
			.height = -static_cast<float>(pass.viewport_height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		VkRect2D sci
		{
			.offset = {0, 0},
			.extent =
			{
				.width = pass.viewport_width,
				.height = pass.viewport_height
			}
		};

		vkCmdSetViewport(frame.cmds, 0, 1, &vp);
		vkCmdSetScissor(frame.cmds, 0, 1, &sci);
		// maybe bind index buffer
		// draw[_indexed]/draw_[indexed_]indirect/draw_[indexed_]indirect_count
		vkCmdDraw(frame.cmds, 3, 1, 0, 0); // todo: dont hardcode this IDIOT
		vkCmdEndRendering(frame.cmds);
		if(render_into_system_image)
		{
			VkImageCopy cpy
			{
				.srcSubresource = VkImageSubresourceLayers
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.srcOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
				.dstSubresource = VkImageSubresourceLayers
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.dstOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
				.extent = VkExtent3D{
					.width = swapchain_width,
					.height = swapchain_height,
					.depth = 1,
				}
			};
			// do *not* use this index into the swapchain images. id is the n'th frame in flight with respect to frame_overlap.
			// the index we actually want to use is the recently acquired swapchain image (which isnt being done yet)
			vkCmdCopyImage(frame.cmds, system_image, VK_IMAGE_LAYOUT_GENERAL, swapchain_images[id], VK_IMAGE_LAYOUT_GENERAL, 1, &cpy);
		}
		// last: transition swapchain image layout from colour attachment (OR general) to present if we need to present the image next.
		return tz::error_code::success;
	}

	bool impl_try_allocate_descriptors(VkDescriptorPool pool, std::span<VkDescriptorSet> sets)
	{
		VkDescriptorSetAllocateInfo alloc
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = pool,
			.descriptorSetCount = frame_overlap,
			.pSetLayouts = set_layouts.data()
		};
		VkResult res = vkAllocateDescriptorSets(current_device, &alloc, sets.data());
		switch(res)
		{
			case VK_SUCCESS: return true; break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("ran out of ram while trying to allocate descriptors...");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("ran out of vram while trying to allocate descriptors...");
			break;
			default:
				tz_error("undocumented error occurred. big bad");
			break;
			case VK_ERROR_FRAGMENTED_POOL:
			[[fallthrough]];
			case VK_ERROR_OUT_OF_POOL_MEMORY:
				return false;
			break;
		}
		return true;
	}
	
	void impl_new_pool()
	{
		VkDescriptorPool& newpool = descriptor_pools.emplace_back();
		VkDescriptorPoolSize image_limit
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = max_image_count_per_pass
		};
		VkDescriptorPoolCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = frame_overlap * 256,
			.poolSizeCount = 1,
			.pPoolSizes = &image_limit
		};
		VkResult res = vkCreateDescriptorPool(current_device, &create, nullptr, &newpool);
		tz_assert(res == VK_SUCCESS, "ruh roh raggy");
	}

	void impl_populate_descriptors(pass_handle passh)
	{
		auto& pass = passes[passh.peek()];
		if(descriptor_pools.empty())
		{
			impl_new_pool();
		}
		VkDescriptorPool pool = descriptor_pools.back();
		while(!impl_try_allocate_descriptors(pool, pass.descriptor_sets))
		{
			impl_new_pool();
			pool = descriptor_pools.back();
		}
		constexpr std::uint32_t image_array_descriptor_binding = 0;
		std::vector<VkDescriptorImageInfo> image_writes;
		std::array<VkWriteDescriptorSet, frame_overlap> descriptor_writes;
		image_writes.reserve(pass.info.resources.size());
		std::size_t image_count = 0;
		for(std::size_t j = 0; j < frame_overlap; j++)
		{
			for(std::size_t i = 0; i < pass.info.resources.size(); i++)
			{
				tz::gpu::resource_handle resh = pass.info.resources[i];
				const auto& res = resources[resh.peek()];
				if(res.is_image())
				{
					image_writes.push_back
					({
						.sampler = res.img_sampler, // todo: samplers.
						.imageView = res.img_view, // todo: image view
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					});
					if(j == 0)
					{
						image_count++;
					}
				}
			}
			descriptor_writes[j] = VkWriteDescriptorSet
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = pass.descriptor_sets[j],
				.dstBinding = image_array_descriptor_binding,
				.dstArrayElement = 0,
				.descriptorCount = static_cast<std::uint32_t>(image_count),
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = image_writes.data() + (j * image_count),
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr
			};
		}
		if(image_count == 0)
		{
			// no need to update anything if tere's no images at all.
			return;
		}
		vkUpdateDescriptorSets(current_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
	}

	VkPipelineLayout impl_create_layout()
	{
		VkDescriptorSetLayoutBinding images_binding
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = max_global_image_count,
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

	tz::error_code impl_validate_colour_targets(pass_info& info, pass_data& pass)
	{
		// set our viewport width/height to that of the first colour target, but assert that *all* colour targets have those same exact dimensions.
		for(std::size_t i = 0; i < info.graphics.colour_targets.size(); i++)
		{
			auto target = info.graphics.colour_targets[i];
			auto [w, h] = impl_get_image_dimensions(target);
			if(target != window_resource && target != tz::nullhand)
			{
				const auto& res = resources[target.peek()];
				if(!res.is_image())
				{
					RETERR(tz::error_code::precondition_failure, "colour target {} of pass \"{}\" is neither tz::gpu::window_resource nor a valid image resource. colour targets must consist of a valid image resource that is viable as a colour target, or the window resource.", i, pass.info.name);
				}
				else
				{
					const auto& img = std::get<image_info>(res.res);
					if(!(img.flags & image_flag::colour_target))
					{
						RETERR(tz::error_code::precondition_failure, "while colour target {} of pass \"{}\" is a valid image resource, specifying it as a colour target is invalid as it was not created with the \"colour_target\" flag.", i, pass.info.name);
					}
				}
			}
			if(i == 0)
			{
				pass.viewport_width = w;
				pass.viewport_height = h;
			}
			else
			{
				if(pass.viewport_width != w)
				{
					RETERR(tz::error_code::precondition_failure, "width of {}'th colour target of pass {} ({}) does not match the rest ({}). all colour targets within a single pass *must* be valid image resources, all with the exact same dimensions.", i, info.name, w, pass.viewport_width);
				}
				if(pass.viewport_height != h)
				{
					RETERR(tz::error_code::precondition_failure, "height of {}'th colour target of pass {} ({}) does not match the rest ({}). all colour targets within a single pass *must* be valid image resources, all with the exact same dimensions.", i, info.name, h, pass.viewport_height);
				}
			}
		}
		return tz::error_code::success;
	}

	void impl_destroy_system_images()
	{
		if(system_image != VK_NULL_HANDLE)
		{
			vkDestroyImageView(current_device, system_image_view, nullptr);
			vmaDestroyImage(alloc, system_image, system_image_mem);
		}
		if(system_depth_image)
		{
			vkDestroyImageView(current_device, system_depth_image_view, nullptr);
			vmaDestroyImage(alloc, system_depth_image, system_depth_image_mem);
		}
	}
}
#endif