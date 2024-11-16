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
	struct settings_t
	{
		bool vsync_enabled = false;
	} settings;

	VkInstance current_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
	VkDevice current_device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	unsigned int swapchain_width = -1; unsigned int swapchain_height = -1;
	std::vector<VkImage> swapchain_images = {};
	std::vector<VkImageView> swapchain_views = {};
	hardware current_hardware = {};
	#define VULKAN_API_VERSION_USED VK_API_VERSION_1_3
	constexpr VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
	VmaAllocator alloc = VK_NULL_HANDLE;
	VkQueue graphics_compute_queue = VK_NULL_HANDLE;
	VkPipelineLayout default_layout = VK_NULL_HANDLE;
	constexpr std::uint32_t max_global_image_count = 8192;
	constexpr std::uint32_t max_image_count_per_pass = 4096;

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
		std::vector<resource_handle> colour_targets;
		std::vector<resource_handle> resources;
		std::size_t image_count = 0;
		VkPipelineLayout layout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkBuffer meta_buffer = VK_NULL_HANDLE;
		VmaAllocation meta_buffer_mem = VK_NULL_HANDLE;
		VkEvent on_finish = VK_NULL_HANDLE;
		std::array<VkDescriptorSet, frame_overlap> descriptor_sets = {};
		std::uint32_t viewport_width = 0;
		std::uint32_t viewport_height = 0;
	};
	std::vector<pass_data> passes = {};

	struct graph_data
	{
		struct entry{tz::hanval han; bool is_graph = false;};
		std::vector<entry> timeline = {};
		std::vector<std::vector<entry>> dependencies = {};
		void(*on_execute)(graph_handle) = nullptr;
		std::string name;
	};
	std::vector<graph_data> graphs = {};

	#if TOPAZ_DEBUG
	const char* validation_layers = "VK_LAYER_KHRONOS_validation";
	#else
	const char* validation_layers = nullptr;
	#endif

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

	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

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
	VKAPI_PTR VkBool32 impl_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);
	std::pair<std::uint32_t, std::uint32_t> impl_get_image_dimensions(tz::gpu::resource_handle imagey_resource);
	tz::error_code impl_need_swapchain(std::uint32_t w, std::uint32_t h);
	void impl_force_new_swapchain();
	VkFormat impl_get_format_from_image_type(tz::gpu::image_type type);
	tz::error_code impl_validate_colour_targets(tz::gpu::pass_info& pinfo, pass_data& data);
	tz::error_code impl_cmd_resource_write(VkCommandBuffer cmds, resource_handle resource, std::span<const std::byte> newdata, std::size_t offset = 0);
	void impl_write_all_resources(pass_handle pass);
	void impl_write_single_resource(resource_handle resh);
	tz::error_code impl_record_gpu_work(pass_handle pass, std::size_t i, std::span<const pass_handle> deps);
	void impl_pass_go(pass_handle pass);
	void impl_destroy_system_images();
	void impl_check_for_resize();
	bool impl_graph_will_present(graph_handle graphh);
	bool impl_graph_writes_to_system_image(graph_handle graphh);
	void impl_execute_subgraph(graph_handle graphh, std::size_t image_index);

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
			.enabledLayerCount = validation_layers != nullptr ? 1u : 0u,
			.ppEnabledLayerNames = validation_layers != nullptr ? &validation_layers : nullptr,
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

		#if TOPAZ_DEBUG
			vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(current_instance, "vkSetDebugUtilsObjectNameEXT"));
			vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(current_instance, "vkCmdBeginDebugUtilsLabelEXT"));
			vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(current_instance, "vkCmdEndDebugUtilsLabelEXT"));

			vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(current_instance, "vkCreateDebugUtilsMessengerEXT"));
			vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(current_instance, "vkDestroyDebugUtilsMessengerEXT"));

			VkDebugUtilsMessengerCreateInfoEXT debug_create
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.pNext = nullptr,
				.flags = 0,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
				.pfnUserCallback = impl_debug_callback,
				.pUserData = nullptr
			};
			vkCreateDebugUtilsMessengerEXT(current_instance, &debug_create, nullptr, &debug_messenger);
		#endif
	}

	void terminate()
	{
		if(current_device != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(current_device);
		}

		for(std::size_t i = 0; i < passes.size(); i++)
		{
			destroy_pass(static_cast<tz::hanval>(i));
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
		#if TOPAZ_DEBUG
			if(debug_messenger != VK_NULL_HANDLE)
			{
				vkDestroyDebugUtilsMessengerEXT(current_instance, debug_messenger, nullptr);
				debug_messenger = VK_NULL_HANDLE;
			}
		#endif
		vkDestroyInstance(current_instance, nullptr);
		current_hardware = {};
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
		if(current_hardware == hw)
		{
			return tz::error_code::success;
		}
		if(current_hardware != hardware{})
		{
			RETERR(tz::error_code::engine_bug, "I have not yet implemented the ability to select hardware twice for two different pieces of hardware. You previously selected \"{}\" but now want \"{}\"", current_hardware.name, hw.name);
		}
		if(hw.caps == hardware_capabilities::neither)
		{
			// incompatible hardware.
			RETERR(error_code::machine_unsuitable, "Graphics hardware {} is not suitable because it supports neither graphics nor compute operations.", hw.name);
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
				RETERR(tz::error_code::machine_unsuitable, "the hardware requested for use is not suitable due to a missing vulkan extension");
			break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				switch(hw.features)
				{
					case tz::gpu::hardware_feature_coverage::ideal:
						RETERR(tz::error_code::engine_bug, "vulkan driver says not all features were available for the device, even though the engine said it was suitable. please submit a bug report.");
					break;
					default:
						RETERR(tz::error_code::machine_unsuitable, "the hardware requested for use is not suitable due to a missing vulkan feature");
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

		#if TOPAZ_DEBUG
			std::string hardware_name = std::format("Hardware: {}", hw.name);
			VkDebugUtilsObjectNameInfoEXT name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(pdev)),
				.pObjectName = hardware_name.c_str()
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &name);

			std::string device_name = std::format("Logical Device: {}", hw.name);
			VkDebugUtilsObjectNameInfoEXT device_name_info
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_DEVICE,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(current_device)),
				.pObjectName = device_name.c_str()
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &device_name_info);
		#endif


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
		tz_assert(hardware_in_use(), "A call to get_used_hardware() was invalid because a piece of hardware has seemingly not yet been selected in a previous call to use_hardware(...). Did you forget to use a specific hardware component?");
		return current_hardware;
	}

	bool hardware_in_use()
	{
		return current_device != VK_NULL_HANDLE;
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

		if(info.flags & buffer_flag::index)
		{
			UNERR(tz::error_code::engine_bug, "index buffers are not yet implemented.");
		}
		if(info.flags & buffer_flag::draw)
		{
			UNERR(tz::error_code::engine_bug, "draw buffers are not yet implemented.");
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
		if(info.flags & buffer_flag::dynamic_access)
		{
			alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}
		else
		{
			alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			// add transfer_dst because static resources will need a transfer command to set their data.
			create.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		VmaAllocationInfo alloc_result;
		VkResult ret = vmaCreateBuffer(alloc, &create, &alloc_info, &res.buf, &res.mem, &alloc_result);
		if(info.flags & buffer_flag::dynamic_access)
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

		#if TOPAZ_DEBUG
			VkDebugUtilsObjectNameInfoEXT debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_BUFFER,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(res.buf)),
				.pObjectName = info.name
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &debug_name);
		#endif

		return static_cast<tz::hanval>(hanval);
	}

	std::expected<resource_handle, tz::error_code> create_image(image_info info)
	{
		if(info.data.size_bytes() == 0)
		{
			UNERR(tz::error_code::invalid_value, "image resource ({}x{}) was not provided any data", info.width, info.height);
		}
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
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &current_hardware.internals.i1,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		create.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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

		#if TOPAZ_DEBUG
			VkDebugUtilsObjectNameInfoEXT image_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(res.img)),
				.pObjectName = info.name
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &image_debug_name);

			std::string view_name = std::format("Image View: {}", info.name);
			VkDebugUtilsObjectNameInfoEXT view_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(res.img_view)),
				.pObjectName = view_name.c_str()
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &view_debug_name);

			std::string sampler_name = std::format("Sampler: {}", info.name);
			VkDebugUtilsObjectNameInfoEXT sampler_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_SAMPLER,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(res.img_sampler)),
				.pObjectName = sampler_name.c_str()
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &sampler_debug_name);
		#endif

		return static_cast<tz::hanval>(hanval);
	}

	tz::error_code destroy_resource(resource_handle res)
	{
		if(res == tz::nullhand)
		{
			RETERR(tz::error_code::invalid_value, "Attempt to destroy a null resource");
		}
		if(res == tz::gpu::window_resource)
		{
			RETERR(tz::error_code::invalid_value, "Attempt to destroy the window resource");
		}
		auto& info = resources[res.peek()];
		for(std::size_t i = 0; i < passes.size(); i++)
		{
			const auto& pass = passes[i];
			auto iter = std::find(pass.info.resources.begin(), pass.info.resources.end(), res);
			if(iter != pass.info.resources.end())
			{
				RETERR(tz::error_code::concurrent_usage, "Deletion of resource {} requested, but this resource is used by pass {}", res.peek(), i);
			}
		}
		if(info.is_invalid())
		{
			RETERR(tz::error_code::invalid_value, "invalid resource handle - either the handle is garbage or you have already destroyed it.");
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

	tz::error_code resource_write(resource_handle resh, std::span<const std::byte> new_data, std::size_t offset)
	{
		auto& res = resources[resh.peek()];
		// update the resource data cpu-side.
		if(offset + new_data.size_bytes() > res.data.size())
		{
			RETERR(tz::error_code::invalid_value, "resource write at offset {} of size {} (total {}) is too large - the resource data is only of size {}", offset, new_data.size_bytes(), (offset + new_data.size_bytes()), res.data.size());
		}
		tz_assert(offset + new_data.size_bytes() <= res.data.size(), "resource write data span is of wrong size.");
		std::copy(new_data.begin(), new_data.end(), res.data.begin() + offset);
		// make sure the change is resident GPU-side.
		// definitely could cause gpu sync issues if commands are currently in-flight that are reading from it.
		impl_write_single_resource(resh);
		return tz::error_code::success;
	}

	std::size_t resource_size(resource_handle resh)
	{
		return resources[resh.peek()].data.size();
	}

	unsigned int image_get_width(resource_handle res)
	{
		if(res == tz::nullhand)
		{
			return 0u;
		}
		if(res == tz::gpu::window_resource)
		{
			return swapchain_width;
		}
		const auto& resource = resources[res.peek()];
		if(!resource.is_image())
		{
			return 0u;
		}
		return std::get<image_info>(resource.res).width;
	}

	unsigned int image_get_height(resource_handle res)
	{
		if(res == tz::nullhand)
		{
			return 0u;
		}
		if(res == tz::gpu::window_resource)
		{
			return swapchain_height;
		}
		const auto& resource = resources[res.peek()];
		if(!resource.is_image())
		{
			return 0u;
		}
		return std::get<image_info>(resource.res).height;
	}

	std::span<const std::byte> resource_read(resource_handle resh)
	{
		auto& res = resources[resh.peek()];
		return res.data;
	}

	void buffer_resize(resource_handle bufh, std::size_t new_size_bytes)
	{
		(void)bufh;
		(void)new_size_bytes;
		tz_error("buffer resize NYI");
	}

	void image_resize(resource_handle imgh, unsigned int new_width, unsigned int new_height)
	{
		(void)imgh;
		(void)new_width; (void)new_height;
		tz_error("image resize NYI");
	}

	tz::error_code index_buffer_write(resource_handle index_buffer, std::span<const index_t> indices)
	{
		return resource_write(index_buffer, std::as_bytes(indices));
	}

	static_assert(sizeof(draw_t) == sizeof(VkDrawIndirectCommand));
	tz::error_code draw_buffer_write(resource_handle draw_buffer, std::uint32_t count, std::span<const draw_t> draws)
	{
		std::vector<std::byte> mem(sizeof(std::uint32_t) + draws.size_bytes());
		auto cursor = mem.data();
		*reinterpret_cast<std::uint32_t*>(cursor) = count;
		std::memcpy(cursor + sizeof(std::uint32_t), draws.data(), draws.size_bytes());
		return resource_write(draw_buffer, mem);
	}

	static_assert(sizeof(draw_indexed_t) == sizeof(VkDrawIndexedIndirectCommand));
	tz::error_code draw_buffer_indexed_write(resource_handle draw_buffer, std::uint32_t count, std::span<const draw_indexed_t> draws)
	{
		std::vector<std::byte> mem(sizeof(std::uint32_t) + draws.size_bytes());
		auto cursor = mem.data();
		*reinterpret_cast<std::uint32_t*>(cursor) = count;
		std::memcpy(cursor + sizeof(std::uint32_t), draws.data(), draws.size_bytes());
		return resource_write(draw_buffer, mem);
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
			UNERR(tz::error_code::invalid_value, "no shader program provided when creating pass. you must provide a valid shader program.");
		}
		if(info.graphics.index_buffer != tz::nullhand)
		{
			UNERR(tz::error_code::engine_bug, "support for index buffers is not yet implemented.");
		}
		if(info.graphics.draw_buffer != tz::nullhand)
		{
			UNERR(tz::error_code::engine_bug, "support for draw buffers is not yet implemented.");
		}
		std::size_t ret_id = passes.size();
		auto& pass = passes.emplace_back();
		std::size_t buffer_count = 0;
		pass.resources.reserve(info.resources.size());
		for(resource_handle resh : info.resources)
		{
			if(resh == tz::nullhand)
			{
				UNERR(tz::error_code::invalid_value, "One of the resources passed into creation of pass {} was the null resource. You cannot use the null resource within a pass.", info.name);
			}
			if(resh == tz::gpu::window_resource)
			{
				UNERR(tz::error_code::invalid_value, "One of the resources passed into creation of pass {} was the window resource. You cannot use the window resource within a pass.", info.name);
			}
			// todo: assert not widnow resouces or nullhand
			const auto& res = resources[resh.peek()];
			if(res.is_buffer())
			{
				buffer_count++;
			}
			else if(res.is_image())
			{
				pass.image_count++;
			}
			pass.resources.push_back(resh);
		}
		pass.info.resources = pass.resources;
		// the meta buffer must always exist. if we have no buffer resources, then it just sits at a single byte.
		VkBufferCreateInfo meta_create
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = buffer_count > 0 ? sizeof(VkDeviceAddress) * buffer_count : 1,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &current_hardware.internals.i1,
		};
		VmaAllocationCreateInfo meta_alloc_create
		{
			.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
		};
		VkResult meta_result = vmaCreateBuffer(alloc, &meta_create, &meta_alloc_create, &pass.meta_buffer, &pass.meta_buffer_mem, nullptr);
		tz_assert(meta_result == VK_SUCCESS, "internal error - failed to create meta buffer.");

		auto top_part = (info.shader.peek() >> 16) & 0xFFFFFFFF;
		auto& shader1 = shaders[--top_part];
		auto bottom_part = info.shader.peek() & 0x0000FFFF;

		VkResult res = VK_SUCCESS;

		if(shader1.ty == shader_type::compute)
		{
			if(current_hardware.caps == hardware_capabilities::graphics_only || current_hardware.caps == hardware_capabilities::neither)
			{
				UNERR(tz::error_code::machine_unsuitable, "Attempt to create compute pass using hardware {} which does not support compute operations.", current_hardware.name);
			}
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
				UNERR(tz::error_code::invalid_value, "provided a shader program consisting of only 1 shader, and that shader is not a compute shader. a graphics shader program must be comprised of both a vertex shader and fragment shader");
			}
			auto& shader2 = shaders[--bottom_part];

			if(current_hardware.caps == hardware_capabilities::compute_only || current_hardware.caps == hardware_capabilities::neither)
			{
				UNERR(tz::error_code::machine_unsuitable, "Attempt to create graphics pass using hardware {} which does not support graphics operations.", current_hardware.name);
			}

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
				UNERR(tz::error_code::invalid_value, "detected graphics pass with no colour targets. a graphics pass must have *at least* one colour target");
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
		// store copy of colour targets as the span could go out of scope soon.
		pass.colour_targets.resize(info.graphics.colour_targets.size());
		std::copy(info.graphics.colour_targets.begin(), info.graphics.colour_targets.end(), pass.colour_targets.begin());
		pass.info.graphics.colour_targets = pass.colour_targets;

		pass.layout = default_layout;
		pass_handle ret = static_cast<tz::hanval>(ret_id);
		impl_write_all_resources(ret);
		impl_populate_descriptors(ret);

		VkEventCreateInfo evt_create
		{
			.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_EVENT_CREATE_DEVICE_ONLY_BIT
		};
		vkCreateEvent(current_device, &evt_create, nullptr, &pass.on_finish);

		return ret;
	}

	void pass_set_triangle_count(pass_handle graphics_pass, std::size_t triangle_count)
	{
		passes[graphics_pass.peek()].info.graphics.triangle_count = triangle_count;
	}

	void pass_set_kernel(pass_handle compute_pass, tz::v3u kernel)
	{
		passes[compute_pass.peek()].info.compute.kernel = kernel;
	}

	void pass_set_scissor(pass_handle graphics_pass, tz::v4u scissor)
	{
		passes[graphics_pass.peek()].info.graphics.scissor = scissor;
	}

	tz::error_code pass_add_image_resource(pass_handle pass, resource_handle res)
	{
		// wait for all shit to be done.
		vkDeviceWaitIdle(current_device);

		if(res == tz::nullhand || res == tz::gpu::window_resource)
		{
			RETERR(tz::error_code::invalid_value, "invalid resource handle ({}) passed to pass_add_image_resource. you can only add proper image resources to a pass.", res == tz::nullhand ? "null" : "window resource");
		}
		const auto& img = resources[res.peek()];
		if(img.is_invalid())
		{
			RETERR(tz::error_code::invalid_value, "invalid resource handle passed to pass_add_image_resource. you can only add a valid image resource to a pass.");
		}
		else if(img.is_buffer())
		{
			RETERR(tz::error_code::invalid_value, "buffer resource passed to pass_add_image_resource. you can only add image resources to a pass, the buffer resources can never change unless you create a new pass.");
		}

		auto& passdata = passes[pass.peek()];

		if(passdata.image_count >= max_image_count_per_pass)
		{
			RETERR(tz::error_code::driver_hazard, "attempted to add a new image resource to pass {}, but that pass already has the maximum number of image resources possible ({})", pass.peek(), max_image_count_per_pass);
		}

		passdata.resources.push_back(res);
		passdata.info.resources = passdata.resources;
		passdata.image_count++;

		// need to write a new descriptor image as it's there now.
		impl_write_single_resource(res);
		impl_populate_descriptors(pass);
		return tz::error_code::success;
	}

	void destroy_pass(pass_handle pass)
	{
		auto i = pass.peek();
		tz_assert(passes.size() > i, "Dodgy handle (value {}) passed to destroy_pass", i);
		vkDestroyPipeline(current_device, passes[i].pipeline, nullptr);
		vmaDestroyBuffer(alloc, passes[i].meta_buffer, passes[i].meta_buffer_mem);
		vkDestroyEvent(current_device, passes[i].on_finish, nullptr);
		passes[i] = {};
	}

	graph_handle create_graph(const char* name)
	{
		std::size_t ret_id = graphs.size();
		auto& graph = graphs.emplace_back();
		graph.name = name;
		return static_cast<tz::hanval>(ret_id);
	}

	void graph_add_pass(graph_handle graphh, pass_handle pass, std::span<const pass_handle> dependencies)
	{
		auto& graph = graphs[graphh.peek()];
		graph.timeline.push_back({.han = static_cast<tz::hanval>(pass), .is_graph = false});
		auto& deps = graph.dependencies.emplace_back();
		deps.resize(dependencies.size());
		std::transform(dependencies.begin(), dependencies.end(), deps.begin(), [](pass_handle h)->graph_data::entry{return {.han = static_cast<tz::hanval>(h), .is_graph = false};});
	}

	void graph_add_subgraph(graph_handle graphh, graph_handle subgraph)
	{
		auto& graph = graphs[graphh.peek()];
		graph.timeline.push_back({.han = static_cast<tz::hanval>(subgraph), .is_graph = true});
		graph.dependencies.push_back({});
	}

	void graph_set_execute_callback(graph_handle graphh, void(*on_execute)(graph_handle))
	{
		graphs[graphh.peek()].on_execute = on_execute;
	}
	
	void execute(graph_handle graphh)
	{
		impl_check_for_resize();
		if(swapchain_width == 0 || swapchain_height == 0)
		{
			return;
		}
		const auto& frame = frames[current_frame];
		const bool will_present = impl_graph_will_present(graphh);
		const bool writes_to_system_image = impl_graph_writes_to_system_image(graphh);

		std::uint32_t image_index = -1u;
		if(will_present)
		{
			vkAcquireNextImageKHR(current_device, swapchain, std::numeric_limits<std::uint64_t>::max(), VK_NULL_HANDLE, frame.swapchain_fence, &image_index);
			vkWaitForFences(current_device, 1, &frame.swapchain_fence, VK_TRUE, std::numeric_limits<std::uint32_t>::max());
			vkResetFences(current_device, 1, &frame.swapchain_fence);
		}

		std::uint64_t begin_point;
		// todo: ec
		// whats the value of our semaphore when the frame starts?
		// this is our new 0
		vkGetSemaphoreCounterValue(current_device, frame.timeline_sem, &begin_point);

		VkImageMemoryBarrier barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = VK_NULL_HANDLE,
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		if(will_present)
		{
			barrier.image = swapchain_images[image_index];
		}

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
		// TODOOO: respect graph dependencies. probably by vkCmdSetEvent and vkCmdWaitEvent?
		if(will_present)
		{
			// transition swapchain image to TRANSFER_DST
			vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		// do stuff
		impl_execute_subgraph(graphh, image_index);

		if(writes_to_system_image)
		{
			tz_assert(image_index != -1u, "ruh roh");
			VkImageBlit blit
			{
				.srcSubresource = VkImageSubresourceLayers
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.srcOffsets =
				{
					VkOffset3D{0, 0, 0},
					VkOffset3D{static_cast<std::int32_t>(swapchain_width), static_cast<std::int32_t>(swapchain_height), 1},
				},
				.dstSubresource = VkImageSubresourceLayers
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.dstOffsets =
				{
					VkOffset3D{0, 0, 0},
					VkOffset3D{static_cast<std::int32_t>(swapchain_width), static_cast<std::int32_t>(swapchain_height), 1},
				},
			};
			// we're about to blit the system image into the swapchain image.
			// however we've just rendered into the system image, so its layout must be color_attachment.
			// to do the transfer we must first transition the system image to transfer_src.
			// we can assume the swapchain image is already in transfer_dst.
			VkImageMemoryBarrier system_image_transfer
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = system_image,
				.subresourceRange = VkImageSubresourceRange
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};
			vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &system_image_transfer);
			vkCmdBlitImage(frame.cmds, system_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain_images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);
		}

		if(will_present)
		{
			// transition swapchain image to PRESENT
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_NONE_KHR;
			vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

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
			.signalSemaphoreCount = will_present ? 1u : 0u,
			.pSignalSemaphores = will_present ? &frame.swapchain_sem : nullptr
		};
		vkQueueSubmit(graphics_compute_queue, 1, &submit, frame.swapchain_fence);
		vkWaitForFences(current_device, 1, &frame.swapchain_fence, VK_TRUE, std::numeric_limits<std::uint32_t>::max());
		vkResetFences(current_device, 1, &frame.swapchain_fence);

		VkResult res;
		if(will_present)
		{
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
		}

		current_frame = (current_frame + 1) % frame_overlap;
	}

	bool settings_get_vsync()
	{
		return settings.vsync_enabled;
	}

	void settings_set_vsync(bool enabled)
	{
		if(settings.vsync_enabled == enabled)
		{
			return;
		}
		settings.vsync_enabled = enabled;
		impl_force_new_swapchain();
	}

	/////////////////// chunky impl IMPLEMENTATION ///////////////////

	VKAPI_PTR VkBool32 impl_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback, void* usrdata)
	{
		(void)type;
		(void)usrdata;
		if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			tz_error("[Vulkan Error Callback]: {}\n", callback->pMessage);
		}
		else if(severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			tz_error("[Vulkan Warning Callback]: {}\n", callback->pMessage);
		}		
		return VK_FALSE;
	}

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
			.presentMode = settings.vsync_enabled ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR,
			.clipped = VK_FALSE,
			.oldSwapchain = swapchain
		};
		VkSwapchainKHR old_swapchain = swapchain;
		VkResult res = vkCreateSwapchainKHR(current_device, &create, nullptr, &swapchain);
		if(old_swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(current_device, old_swapchain, nullptr);
		}
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
				return tz::error_code::machine_unsuitable;
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
		for(std::size_t i = 0; i < swapchain_images.size(); i++)
		{
			VkImage swapchain_img = swapchain_images[i];
			// make copy and point to swapchain image.
			view_create.image = swapchain_img;
			VkImageView& view = swapchain_views.emplace_back();
			vkCreateImageView(current_device, &view_create, nullptr, &view);

			#if TOPAZ_DEBUG
				std::string view_name = std::format("Image View: Swapchain Image {}", i);
				VkDebugUtilsObjectNameInfoEXT debug_name
				{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = nullptr,
					.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
					.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(view)),
					.pObjectName = view_name.c_str()
				};
				vkSetDebugUtilsObjectNameEXT(current_device, &debug_name);
			#endif
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

		#if TOPAZ_DEBUG
			VkDebugUtilsObjectNameInfoEXT sys_image_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(system_image)),
				.pObjectName = "System Image"
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &sys_image_debug_name);

			VkDebugUtilsObjectNameInfoEXT sys_view_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(system_image_view)),
				.pObjectName = "System Image View"
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &sys_view_debug_name);

			VkDebugUtilsObjectNameInfoEXT sys_depth_image_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(system_depth_image)),
				.pObjectName = "System Depth Image"
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &sys_depth_image_debug_name);

			VkDebugUtilsObjectNameInfoEXT sys_depth_view_debug_name
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = nullptr,
				.objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
				.objectHandle = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(system_depth_image_view)),
				.pObjectName = "System Depth Image View"
			};
			vkSetDebugUtilsObjectNameEXT(current_device, &sys_depth_view_debug_name);
		#endif

		return tz::error_code::partial_success;
	}

	void impl_force_new_swapchain()
	{
		auto w = swapchain_width;
		auto h = swapchain_height;
		swapchain_width = 0;
		swapchain_height = 0;
		impl_need_swapchain(w, h);
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

			if(buffer.flags & buffer_flag::dynamic_access)
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
				},
				.imageOffset = VkOffset3D{},
				.imageExtent = VkExtent3D{.width = image.width, .height = image.height, .depth = 1}
			};
			VkImageMemoryBarrier barrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_NONE,
				.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
			vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_NONE, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			vkCmdCopyBufferToImage(cmds, scratchbuf.buf, res.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpy);
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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

	void impl_write_single_resource(resource_handle resh)
	{
		const auto& res = resources[resh.peek()];	
		if(res.is_buffer())
		{
			const auto& info = std::get<buffer_info>(res.res);
			if(info.flags & buffer_flag::dynamic_access)
			{
				std::memcpy(res.buffer_mapped_address, res.data.data(), res.data.size());
				return;
			}
		}
		
		VkCommandBufferBeginInfo create
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		// todo: error checking.
		vkBeginCommandBuffer(scratch_cmds, &create);
		tz_must(impl_cmd_resource_write(scratch_cmds, resh, res.data, 0));

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
		vkResetFences(current_device, 1, &scratch_fence);
		vkResetCommandBuffer(scratch_cmds, 0);
		// no need to write anything to the meta buffer - metabuffer only changes if any of the buffers are *resized* coz the VkBuffer is different and the device address is also different.
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
		const auto& pass = passes[passh.peek()];
		if(pass.info.resources.empty())
		{
			return;
		}
		vkBeginCommandBuffer(scratch_cmds, &create);
		std::vector<VkDeviceAddress> buffer_addresses = {};
		for(const auto& resh : pass.info.resources)
		{
			const auto& res = resources[resh.peek()];	
			if(res.is_buffer())
			{
				const auto& buffer = std::get<buffer_info>(res.res);
				tz_must(impl_cmd_resource_write(scratch_cmds, resh, buffer.data, 0));
				buffer_addresses.push_back(res.buffer_device_address);
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

		if(buffer_addresses.size())
		{
			tz_assert(pass.meta_buffer != VK_NULL_HANDLE, "meta buffer wasnt detected when it should exist (coz we have {} buffers)", buffer_addresses.size());
			scratchbuf_t& scratchbuf = scratch_buffers.emplace_back();
			std::size_t meta_buffer_size = buffer_addresses.size() * sizeof(buffer_addresses[0]);
			VkBufferCreateInfo create
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = meta_buffer_size,
				.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
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
			std::memcpy(alloc_out.pMappedData, buffer_addresses.data(), meta_buffer_size);
			VkBufferCopy cpy
			{
				.srcOffset = 0,
				.dstOffset = 0,
				.size = meta_buffer_size
			};
			vkCmdCopyBuffer(scratch_cmds, scratchbuf.buf, pass.meta_buffer, 1, &cpy);
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
		VkResult res = vkQueueSubmit(graphics_compute_queue, 1, &submit, scratch_fence);	
		tz_assert(res == VK_SUCCESS, "ruh roh");
		vkWaitForFences(current_device, 1, &scratch_fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
		vkResetFences(current_device, 1, &scratch_fence);
	}

	tz::error_code impl_record_compute_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id);
	tz::error_code impl_record_graphics_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id);

	tz::error_code impl_record_gpu_work(pass_handle passh, std::size_t i, std::span<const pass_handle> dependencies)
	{
		const auto& pass = passes[passh.peek()]; 
		tz::error_code ret = tz::error_code::success;
		const auto& frame = frames[i];
		// GPU work goes here.
		auto top_part = (pass.info.shader.peek() >> 16) & 0xFFFFFFFF;
		auto& shader1 = shaders[--top_part];
		vkCmdResetEvent(frame.cmds, pass.on_finish, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		for(pass_handle dep : dependencies)
		{
			vkCmdWaitEvents(frame.cmds, 1, &passes[dep.peek()].on_finish, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
		}
		
		#if TOPAZ_DEBUG
			std::string pass_label_name = std::format("Pass: {}", pass.info.name);
			VkDebugUtilsLabelEXT pass_label
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pNext = nullptr,
				.pLabelName = pass_label_name.c_str()
			};
			vkCmdBeginDebugUtilsLabelEXT(frame.cmds, &pass_label);
		#endif
		if(shader1.ty == shader_type::compute)
		{
			ret = impl_record_compute_work(pass, frame, i);
		}
		else
		{
			ret = impl_record_graphics_work(pass, frame, i);
		}
		#if TOPAZ_DEBUG
			vkCmdEndDebugUtilsLabelEXT(frame.cmds);
		#endif
		vkCmdSetEvent(frame.cmds, pass.on_finish, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		return ret;
	}

	tz::error_code impl_record_compute_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id)
	{
		vkCmdBindPipeline(frame.cmds, VK_PIPELINE_BIND_POINT_COMPUTE, pass.pipeline);
		vkCmdBindDescriptorSets(frame.cmds, VK_PIPELINE_BIND_POINT_COMPUTE, pass.layout, 0u, 1, pass.descriptor_sets.data() + id, 0, nullptr);
		vkCmdDispatch(frame.cmds, pass.info.compute.kernel[0], pass.info.compute.kernel[1], pass.info.compute.kernel[2]);

		return tz::error_code::success;
	}

	tz::error_code impl_record_graphics_work(const pass_data& pass, const frame_data_t& frame, std::uint32_t id)
	{
		std::vector<VkRenderingAttachmentInfo> colour_attachments;
		std::vector<VkImageMemoryBarrier> colour_transitions;
		colour_attachments.reserve(pass.info.graphics.colour_targets.size());
		colour_transitions.reserve(pass.info.graphics.colour_targets.size());
		VkClearColorValue clear_colour{.float32 = {}};
		// BGRA
		clear_colour.float32[0] = pass.info.graphics.clear_colour[0];
		clear_colour.float32[1] = pass.info.graphics.clear_colour[1];
		clear_colour.float32[2] = pass.info.graphics.clear_colour[2];
		clear_colour.float32[3] = pass.info.graphics.clear_colour[3];

		for(const auto colour_resh : pass.info.graphics.colour_targets)
		{
			VkImage rt = VK_NULL_HANDLE;
			VkImageView rtv = VK_NULL_HANDLE;
			if(colour_resh == window_resource)
			{
				rt = system_image;
				rtv = system_image_view;
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
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.resolveImageView = VK_NULL_HANDLE,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp = (pass.info.graphics.flags & graphics_flag::dont_clear) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR, // clear colour target before rendered into
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = VkClearValue{clear_colour}
			});
			if(!(pass.info.graphics.flags & graphics_flag::dont_clear))
			{
				colour_transitions.push_back({
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = VK_ACCESS_NONE,
					.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
			vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, colour_transitions.size(), colour_transitions.data());
		}
		VkImage depth_rt = VK_NULL_HANDLE;
		VkImageView depth_rtv = VK_NULL_HANDLE;
		if(pass.info.graphics.depth_target != tz::nullhand)
		{
			if(pass.info.graphics.depth_target == tz::gpu::window_resource)
			{
				depth_rt = system_depth_image;
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
				depth_rt = depth_image_resource.img;
				depth_rtv = depth_image_resource.img_view;
			}

			VkImageMemoryBarrier depth_barrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_NONE,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = depth_rt,
				.subresourceRange = VkImageSubresourceRange
				{
					.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};
			vkCmdPipelineBarrier(frame.cmds, VK_PIPELINE_STAGE_NONE, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depth_barrier);
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
			.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = (pass.info.graphics.flags & graphics_flag::dont_clear) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // potentially want to store if another pass is going to read from depth afterwards (in the timeline)
			.clearValue =
			{
				.depthStencil = VkClearDepthStencilValue
				{
					.depth = 1.0f
				}}
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
		if(pass.info.graphics.index_buffer != tz::nullhand)
		{
			const auto& indices = resources[pass.info.graphics.index_buffer.peek()];
			if(indices.is_invalid())
			{
				RETERR(tz::error_code::invalid_value, "Graphics pass \"{}\" uses index buffer handle {}, which is an invalid resource.", pass.info.name, pass.info.graphics.index_buffer.peek());
			}
			vkCmdBindIndexBuffer(frame.cmds, indices.buf, 0, VK_INDEX_TYPE_UINT32);
		}
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
		if(pass.info.graphics.scissor != tz::v4u::filled(-1))
		{
			sci.offset = {static_cast<std::int32_t>(pass.info.graphics.scissor[0]), static_cast<std::int32_t>(pass.info.graphics.scissor[1])};
			sci.extent = {static_cast<std::uint32_t>(pass.info.graphics.scissor[2]), static_cast<std::uint32_t>(pass.info.graphics.scissor[3])};
		}

		vkCmdSetViewport(frame.cmds, 0, 1, &vp);
		vkCmdSetScissor(frame.cmds, 0, 1, &sci);

		// draw logic
		VkBuffer draw_buf = VK_NULL_HANDLE;
		std::size_t draw_buf_max_size_indexed = 0;
		std::size_t draw_buf_max_size_unindexed = 0;
		if(pass.info.graphics.draw_buffer != tz::nullhand)
		{
			draw_buf = resources[pass.info.graphics.draw_buffer.peek()].buf;
			draw_buf_max_size_unindexed = (std::get<buffer_info>(resources[pass.info.graphics.draw_buffer.peek()].res).data.size_bytes() - sizeof(std::uint32_t)) / sizeof(VkDrawIndirectCommand);
			draw_buf_max_size_indexed = (std::get<buffer_info>(resources[pass.info.graphics.draw_buffer.peek()].res).data.size_bytes() - sizeof(std::uint32_t)) / sizeof(VkDrawIndexedIndirectCommand);
		}
		if(pass.info.graphics.index_buffer != tz::nullhand)
		{
			if(draw_buf != VK_NULL_HANDLE)
			{
				vkCmdDrawIndexedIndirectCount(frame.cmds, draw_buf, sizeof(std::uint32_t), draw_buf, 0, draw_buf_max_size_indexed, sizeof(VkDrawIndexedIndirectCommand));
			}
			else
			{
				vkCmdDrawIndexed(frame.cmds, pass.info.graphics.triangle_count * 3, 1, 0, 0, 0);
			}
		}
		else
		{
			if(draw_buf != VK_NULL_HANDLE)
			{
				vkCmdDrawIndirectCount(frame.cmds, draw_buf, sizeof(std::uint32_t), draw_buf, 0, draw_buf_max_size_unindexed, sizeof(VkDrawIndirectCommand));
			}
			else
			{
				vkCmdDraw(frame.cmds, pass.info.graphics.triangle_count * 3, 1, 0, 0);
			}
		}
		vkCmdEndRendering(frame.cmds);
		return tz::error_code::success;
	}

	bool impl_try_allocate_descriptors(VkDescriptorPool pool, std::span<VkDescriptorSet> sets, std::size_t image_count)
	{
		std::array<std::uint32_t, frame_overlap> variable_counts;
		std::fill(variable_counts.begin(), variable_counts.end(), image_count);
		VkDescriptorSetVariableDescriptorCountAllocateInfo variable_alloc
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorSetCount = frame_overlap,
			.pDescriptorCounts = variable_counts.data()
		};
		VkDescriptorSetAllocateInfo alloc
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &variable_alloc,
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
		std::size_t image_count = 0;
		for(resource_handle resh : pass.info.resources)
		{
			if(resh == tz::nullhand)
			{
			}
			else if(resh == tz::gpu::window_resource || resources[resh.peek()].is_image())
			{
				image_count++;
			}
		}
		VkDescriptorPool pool = descriptor_pools.back();
		while(!impl_try_allocate_descriptors(pool, pass.descriptor_sets, image_count))
		{
			impl_new_pool();
			pool = descriptor_pools.back();
		}
		constexpr std::uint32_t image_array_descriptor_binding = 1;
		std::vector<VkDescriptorImageInfo> image_writes;
		VkDescriptorBufferInfo meta_buffer_write
		{
			.buffer = pass.meta_buffer,
			.offset = 0,
			.range = VK_WHOLE_SIZE
		};
		image_writes.reserve(pass.info.resources.size() * frame_overlap);
		std::vector<VkWriteDescriptorSet> descriptor_writes;
		// first do meta buffers so they're first in the list.
		for(std::size_t j = 0; j < frame_overlap; j++)
		{
			descriptor_writes.push_back(VkWriteDescriptorSet
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = pass.descriptor_sets[j],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = &meta_buffer_write,
				.pTexelBufferView = nullptr
			});
		}

		for(std::size_t j = 0; j < frame_overlap; j++)
		{
			if(image_count == 0)
			{
				break;
			}
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
				}
			}
			descriptor_writes.push_back(VkWriteDescriptorSet
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
			});
		}
		vkUpdateDescriptorSets(current_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
	}

	VkPipelineLayout impl_create_layout()
	{
		std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
		bindings[0] =
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		};
		bindings[1] = 
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = max_global_image_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		};
		std::array<VkDescriptorBindingFlags, 2> flags{};
		flags[0] = 0;
		flags[1] =
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
			VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo flags_create
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = flags.size(),
			.pBindingFlags = flags.data()
		};
		VkDescriptorSetLayoutCreateInfo dlcreate
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &flags_create,
			.flags =
				VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = bindings.size(),
			.pBindings = bindings.data()
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
					RETERR(tz::error_code::invalid_value, "colour target {} of pass \"{}\" is neither tz::gpu::window_resource nor a valid image resource. colour targets must consist of a valid image resource that is viable as a colour target, or the window resource.", i, pass.info.name);
				}
				else
				{
					const auto& img = std::get<image_info>(res.res);
					if(!(img.flags & image_flag::colour_target))
					{
						RETERR(tz::error_code::invalid_value, "while colour target {} of pass \"{}\" is a valid image resource, specifying it as a colour target is invalid as it was not created with the \"colour_target\" flag.", i, pass.info.name);
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

	void impl_check_for_resize()
	{
		if(!tz::os::window_is_open())
		{
			return;
		}
		auto cur_width = tz::os::window_get_width();
		auto cur_height = tz::os::window_get_height();
		if(cur_width == 0 || cur_height == 0)
		{
			swapchain_width = 0;
			swapchain_height = 0;
		}
		if(cur_width != swapchain_width || cur_height != swapchain_height)
		{
			// window size has changed.
			tz::error_code res = impl_need_swapchain(cur_width, cur_height);
			if(res != tz::error_code::partial_success)
			{
				tz_must(res);
			}
			for(auto& pass : passes)
			{
				tz_must(impl_validate_colour_targets(pass.info, pass));
			}
		}
	}

	bool impl_graph_will_present(graph_handle graphh)
	{
		const auto& graph = graphs[graphh.peek()];
		for(const graph_data::entry& entry : graph.timeline)
		{
			if(entry.is_graph)
			{
				if(impl_graph_will_present(entry.han))
				{
					return true;
				}	
			}
			else
			{
				if(entry.han == tz::gpu::present_pass)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool impl_graph_writes_to_system_image(graph_handle graphh)
	{
		const auto& graph = graphs[graphh.peek()];
		for(const graph_data::entry& entry : graph.timeline)
		{
			if(entry.is_graph)
			{
				if(impl_graph_writes_to_system_image(entry.han))
				{
					return true;
				}	
			}
			else
			{
				if(entry.han == tz::gpu::present_pass)
				{
					continue;
				}
				const auto& pass = passes[static_cast<std::size_t>(entry.han)];
				for(resource_handle colour_target : pass.colour_targets)
				{
					if(colour_target == tz::gpu::window_resource)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	void impl_execute_subgraph(graph_handle graphh, std::size_t image_index)
	{
		auto& graph = graphs[graphh.peek()];
		if(graph.on_execute != nullptr)
		{
			graph.on_execute(graphh);
		}

		#if TOPAZ_DEBUG
			std::string graph_label_name = std::format("Graph: {}", graph.name);
			VkDebugUtilsLabelEXT graph_label
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pNext = nullptr,
				.pLabelName = graph_label_name.c_str()
			};
			vkCmdBeginDebugUtilsLabelEXT(frames[current_frame].cmds, &graph_label);
		#endif

		// go through timeline and record all gpu work.
		// any of these may or may not try to blit (transfer) an image into our current swapchain image.
		for(std::size_t i = 0; i < graph.timeline.size(); i++)
		{
			const auto& entry = graph.timeline[i];
			if(entry.is_graph)
			{
				impl_execute_subgraph(entry.han, image_index);
			}
			else if(entry.han != tz::gpu::present_pass)
			{
				std::span<const graph_data::entry> deps = graph.dependencies[i];
				std::vector<pass_handle> dep_passes;
				dep_passes.reserve(deps.size());
				for(auto& dep_entry : deps)
				{
					tz_assert(!dep_entry.is_graph, "a subgraph cannot be a dependency - only a pass can be a dependency");
					dep_passes.push_back(dep_entry.han);
				}
				tz_must(impl_record_gpu_work(entry.han, current_frame, dep_passes));
			}
		}

		#if TOPAZ_DEBUG
			vkCmdEndDebugUtilsLabelEXT(frames[current_frame].cmds);
		#endif
	}
}
#endif