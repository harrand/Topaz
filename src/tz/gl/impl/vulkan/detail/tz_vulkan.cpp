#if TZ_VULKAN
#include "tz/tz.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#include "tz/gl/impl/vulkan/detail/extensions.hpp"
#include <algorithm>
#include <utility>
#include <cstring>

namespace tz::gl::vk2
{
	VulkanInstance* inst = nullptr;

	void initialise(tz::game_info game_info)
	{
		TZ_PROFZONE("Vulkan Backend - Backend Initialise", 0xFFAA0000);
		tz::assert(inst == nullptr, "Vulkan Backend already initialised");
		inst = new VulkanInstance
		{{
			.game_info = game_info,
#if TZ_DEBUG
			.extensions = {InstanceExtension::DebugMessenger},
#endif
			.window = &tz::window()
		}};
		tz::report("Vulkan v%u.%u Initialised", vulkan_version.major, vulkan_version.minor);
	}

	void terminate()
	{
		tz::assert(inst != nullptr, "Not initialised");
		delete inst;
		inst = nullptr;
		tz::report("Vulkan v%u.%u Terminated", vulkan_version.major, vulkan_version.minor);
	}

	const VulkanInstance& get()
	{
		tz::assert(inst != nullptr, "Not initialised");
		return *inst;
	}

	inline VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		[[maybe_unused]] void* user_data
	)
	{
		if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			tz::error("[Vulkan Error Callback]: %s\n", callback_data->pMessage);
		}
		else if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			tz::report("[Vulkan Warning Callback]: %s\n", callback_data->pMessage);
		}
		else
		{
			// Support for shader asserts (tz::debug::assert(expr) from <debug> in TZSL).
			std::string_view callback_message = callback_data->pMessage;
			auto pos = callback_message.find("TZ_GPUASSERT(");
			if(pos != std::string_view::npos)
			{
				constexpr int expr_loc = 13;
				callback_message.remove_prefix(pos);
				if(callback_message[expr_loc] == '0')
				{
					constexpr int shadertype_loc = 15;
					const char* shadertype;
					auto shadertype_index = static_cast<int>(callback_message[shadertype_loc]) - '0';
					switch(shadertype_index)
					{
						// Note: These must match the order in tzslc::shader_stage.
						case 0:
							shadertype = "Compute";
						break;
						case 1:
							shadertype = "Vertex";
						break;
						case 2:
							shadertype = "Tessellation Control";
						break;
						case 3:
							shadertype = "Tessellation Evaluation";
						break;
						case 4:
							shadertype = "Fragment";
						break;
						default:
							shadertype = "Unknown";
						break;
					}
					tz::error("%s Shader%s", shadertype, callback_message.data() + shadertype_loc + 2);
				}
			}
			else
			{
				if(message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT && message_type == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
				{
					// It could be a debug printf or a normal validation info.
					constexpr char gpumsg_token[] = "TZ_GPUMSG: ";
					auto print_pos = callback_message.find(gpumsg_token);
					if(print_pos != std::string_view::npos)
					{
						// We're printing!
						callback_message.remove_prefix(print_pos + std::strlen(gpumsg_token));
						//callback_message.remove_suffix(callback_message.find("\""));
						tz::report("%s", callback_message.data());
					}
				}
			}
		}
		return VK_FALSE;
	}

	VkResult vkCreateDebugUtilsMessengerEXT
	(
	    VkInstance instance,
	    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	    const VkAllocationCallbacks* pAllocator,
	    VkDebugUtilsMessengerEXT* pMessenger
	)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if(func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void vkDestroyDebugUtilsMessengerEXT
	(
		[[maybe_unused]] VkInstance instance,
		[[maybe_unused]] VkDebugUtilsMessengerEXT messenger,
		[[maybe_unused]] const VkAllocationCallbacks* pAllocator
	)	
	{
		#if TZ_DEBUG
			auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
			if(func != nullptr)
			{
				func(instance, messenger, pAllocator);
			}
		#endif
	}

	VkDebugUtilsMessengerCreateInfoEXT make_debug_messenger_info()
	{
		VkDebugUtilsMessengerCreateInfoEXT create{};
		create.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		create.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		create.pfnUserCallback = default_debug_callback;
		return create;
	}

	VulkanDebugMessenger::VulkanDebugMessenger(const VulkanInstance& instance):
	debug_messenger(VK_NULL_HANDLE),
	instance(instance.native())
	{
		VkDebugUtilsMessengerCreateInfoEXT create = make_debug_messenger_info();
		VkResult res = vk2::vkCreateDebugUtilsMessengerEXT(instance.native(), &create, nullptr, &this->debug_messenger);
		switch(res)
		{
			case VK_SUCCESS:
				// nothing
			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create DebugMessenger because we ran out of memory.");
			break;
			default:
				tz::error("Failed to create DebugMessenger, but couldn't find out how. This unrecognised error code is undocumented by the Vulkan API, so it's probably something very weird. Please submit a bug report!");
			break;
		}
	}

	VulkanDebugMessenger::VulkanDebugMessenger(VulkanDebugMessenger&& move):
	debug_messenger(VK_NULL_HANDLE),
	instance(VK_NULL_HANDLE)
	{
		*this = std::move(move);
	}

	VulkanDebugMessenger::~VulkanDebugMessenger()
	{
		if(this->debug_messenger != VK_NULL_HANDLE)
		{
			vk2::vkDestroyDebugUtilsMessengerEXT(this->instance, this->debug_messenger, nullptr);
			this->debug_messenger = VK_NULL_HANDLE;
		}
	}

	VulkanDebugMessenger& VulkanDebugMessenger::operator=(VulkanDebugMessenger&& rhs)
	{
		std::swap(this->debug_messenger, rhs.debug_messenger);
		std::swap(this->instance, rhs.instance);
		return *this;
	}

	WindowSurface::WindowSurface(const VulkanInstance& instance, const tz::wsi::window& window):
	surface(VK_NULL_HANDLE),
	instance(&instance),
	window(&window)
	{
		//tz::assert(!this->window->is_null(), "Cannot create WindowSurface off of a null window. GLFW has likely failed. Please submit a bug report.");

		this->surface = this->window->make_vulkan_surface(this->instance->native());
		//VkResult res = glfwCreateWindowSurface(this->instance->native(), this->window->get_middleware_handle(), nullptr, &this->surface);
		//switch(res)
		//{
		//	case VK_SUCCESS:

		//	break;
		//	case VK_ERROR_INITIALIZATION_FAILED:
		//		tz::error("Failed to find either a Vulkan loader or a minimally functional ICD (installable client driver), cannot create Window Surface. If you're an end-user, please ensure your drivers are upto-date -- Note that while this is almost certainly *not* a bug, this is a fatal error and the application must crash.");
		//	break;
		//	case VK_ERROR_EXTENSION_NOT_PRESENT:
		//		tz::error("The provided VulkanInstance does not support window surface creation. Please submit a bug report.");
		//	break;
		//	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		//		tz::error("Window was not created to be used for Vulkan (client api hint wasn't set to GLFW_NO_API). Please submit a bug report.");
		//	break;
		//	default:
		//		tz::error("Window surface creation failed, but for an unknown reason. Please ensure your computer meets the minimum requirements for this program. If you are absolutely sure that your machine is valid, please submit a bug report.");
		//	break;
		//}
	}

	WindowSurface::WindowSurface(WindowSurface&& move):
	surface(VK_NULL_HANDLE),
	instance(nullptr),
	window(nullptr)
	{
		*this = std::move(move);
	}

	WindowSurface::~WindowSurface()
	{
		if(this->surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(this->instance->native(), this->surface, nullptr);
		}
	}

	WindowSurface& WindowSurface::operator=(WindowSurface&& rhs)
	{
		std::swap(this->surface, rhs.surface);
		std::swap(this->instance, rhs.instance);
		std::swap(this->window, rhs.window);
		return *this;
	}

	const VulkanInstance& WindowSurface::get_instance() const
	{
		tz::assert(this->instance != nullptr, "WindowSurface had null instance");
		return *this->instance;
	}

	const tz::wsi::window& WindowSurface::get_window() const
	{
		tz::assert(this->window != nullptr, "WindowSurface had nullptr or null tz::Window. Please submit a bug report.");
		return *this->window;
	}

	WindowSurface::NativeType WindowSurface::native() const
	{
		return this->surface;
	}
	

	VulkanInstance::VulkanInstance(VulkanInstanceInfo info):
	info(info),
	instance(VK_NULL_HANDLE),
	maybe_debug_messenger(std::nullopt),
	maybe_window_surface(std::nullopt)
	{
		std::string engine_name_str = info.game_info.to_string();
		VkApplicationInfo app_info
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = info.game_info.name,
			.applicationVersion = util::tz_to_vk_version(info.game_info.version),
			.pEngineName = engine_name_str.c_str(),
			.engineVersion = util::tz_to_vk_version(info.game_info.engine.version),
			.apiVersion = util::tz_to_vk_version(vulkan_version)
		};
		// Validation Layers
		std::span<const char*> enabled_layers;
		[[maybe_unused]] const char* layer = "VK_LAYER_KHRONOS_validation";
		if(TZ_DEBUG && info.enable_validation_layers)
		{
			enabled_layers = {&layer, 1};
		}

		// Extensions (Specified from VulkanInstanceInfo + GLFW)
		util::VkExtensionList extension_natives;
		extension_natives.resize(info.extensions.count() + 2);
		std::transform(info.extensions.begin(), info.extensions.end(), extension_natives.begin(), [](InstanceExtension ext){return util::to_vk_extension(ext);});
		extension_natives[info.extensions.count()] = "VK_KHR_surface";
		// VK_KHR_XLIB_SURFACE_EXTENSION_NAME
		//
		extension_natives[info.extensions.count() + 1] = TZ_WSI_VULKAN_EXTENSION_NAME;
		// Use what we asked for, plus everything GLFW wants.

		void* inst_create_pnext = nullptr;
		// Debug Messenger
		const bool create_debug_validation = info.extensions.contains(InstanceExtension::DebugMessenger) && TZ_DEBUG;
		tz::assert(info.window != nullptr, "Null window provided. Please submit a bug report.");
		VkDebugUtilsMessengerCreateInfoEXT debug_validation_create;
		if(create_debug_validation)
		{
			debug_validation_create = make_debug_messenger_info();
			inst_create_pnext = &debug_validation_create;
		}

		#if TZ_DEBUG
			// If we're on debug, we need to pass VkValidationFeatures to enable debug pritnf.
			VkValidationFeatureEnableEXT validation_features_enabled[1] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
			VkValidationFeaturesEXT validation_features =
			{
				.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
				.pNext = &debug_validation_create,
				.enabledValidationFeatureCount = 1,
				.pEnabledValidationFeatures = validation_features_enabled,
				.disabledValidationFeatureCount = 0,
				.pDisabledValidationFeatures = nullptr
			};
			inst_create_pnext = &validation_features;
		#endif

		VkInstanceCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = inst_create_pnext,
			.flags = 0,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = static_cast<std::uint32_t>(enabled_layers.size()),
			.ppEnabledLayerNames = enabled_layers.data(),
			.enabledExtensionCount = static_cast<std::uint32_t>(extension_natives.length()),
			.ppEnabledExtensionNames = extension_natives.data()
		};
		VkResult res = vkCreateInstance(&create, nullptr, &this->instance);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz::error("Failed to create VulkanInstance because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz::error("Failed to create VulkanInstance because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				tz::error("Failed to create VulkanInstance due to an implementation-specific reason that has not made itself clear. Ensure that your machine supports Vulkan 1.2");
			break;
			case VK_ERROR_LAYER_NOT_PRESENT:
				if(info.enable_validation_layers)
				{
					tz::report("Initial VulkanInstance creation failed due to layer not present (most likely Vulkan SDK is not installed or is corrupt). Retrying without validation layers...");
					info.enable_validation_layers = false;
					*this = {info};
				}
				else
				{
					tz::error("Vulkan Instance creation failed due to layer not present, but `info.enable_validation_layers` was false. Logic error, please submit a bug report.");
				}
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz::error("Failed to create VulkanInstance because one or more of the provided %zu extensions is not supported on this machine. Please submit a bug report.", create.enabledExtensionCount);
			break;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				tz::error("Failed to create VulkanInstance because Vulkan %u.%u is not supported on this machine. You cannot run this build/version of Topaz on this machine unless you update your drivers such that support is achieved. Alternatively have you tried the OpenGL build?", vulkan_version.major, vulkan_version.minor);
			break;
			default:
				tz::error("Failed to create VulkanInstance but cannot determine why. Please submit a bug report.");
			break;
		}
		if(create_debug_validation)
		{
			this->maybe_debug_messenger = VulkanDebugMessenger{*this};
		}
		this->maybe_window_surface = WindowSurface{*this, *info.window};
		this->load_extension_functions();
	}


	VulkanInstance::~VulkanInstance()
	{
		this->maybe_debug_messenger = std::nullopt;	
		this->maybe_window_surface = std::nullopt;

		if(this->instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(this->instance, nullptr);	
			this->instance = VK_NULL_HANDLE;
		}
	}

	bool VulkanInstance::has_surface() const
	{
		return this->maybe_window_surface.has_value();
	}

	bool VulkanInstance::validation_layers_enabled() const
	{
		return this->info.enable_validation_layers;
	}

	const WindowSurface& VulkanInstance::get_surface() const
	{
		tz::assert(this->has_surface(), "VulkanInstance did not have attached surface. Please submit a bug report.");
		return this->maybe_window_surface.value();
	}

	const InstanceExtensionList& VulkanInstance::get_extensions() const
	{
		return this->info.extensions;
	}

	VulkanInstance::NativeType VulkanInstance::native() const
	{
		return this->instance;
	}

	bool VulkanInstance::operator==(const VulkanInstance& rhs) const
	{
		return this->instance == rhs.instance;
	}

	void VulkanInstance::ext_begin_debug_utils_label(VkCommandBuffer cmdbuf_native, VkDebugUtilsLabelEXT label) const
	{
		tz::assert(this->ext_vkcmdbegindebugutilslabel != nullptr && TZ_DEBUG, "vkCmdBeginDebugUtilsLabelEXT failed to load properly. Either you're not on a debug build or your machine does not support debug utils.");
		this->ext_vkcmdbegindebugutilslabel(cmdbuf_native, &label);
	}

	void VulkanInstance::ext_end_debug_utils_label(VkCommandBuffer cmdbuf_native) const
	{
		tz::assert(this->ext_vkcmdenddebugutilslabel != nullptr && TZ_DEBUG, "vkCmdEndDebugUtilsLabelEXT failed to load properly. Either you're not on a debug build or your machine does not support debug utils.");
		this->ext_vkcmdenddebugutilslabel(cmdbuf_native);
	}

	VkResult VulkanInstance::ext_set_debug_utils_object_name(VkDevice device_native, VkDebugUtilsObjectNameInfoEXT info) const
	{
		tz::assert(this->ext_vksetdebugutilsobjectname != nullptr && TZ_DEBUG, "vkSetDebugUtilsObjectNameEXT failed to load properly. Either you're not on a debug build or your machine does not support debug utils.");
		return this->ext_vksetdebugutilsobjectname(device_native, &info);
	}

	void VulkanInstance::load_extension_functions()
	{
		#if TZ_DEBUG
			this->ext_vkcmdbegindebugutilslabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(this->native(), "vkCmdBeginDebugUtilsLabelEXT"));
			this->ext_vkcmdenddebugutilslabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(this->native(), "vkCmdEndDebugUtilsLabelEXT"));
			this->ext_vksetdebugutilsobjectname = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(this->native(), "vkSetDebugUtilsObjectNameEXT"));
		#endif
	}

}

#endif // TZ_VULKAN
