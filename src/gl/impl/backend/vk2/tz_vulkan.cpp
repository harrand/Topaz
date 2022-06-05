#if TZ_VULKAN
#include "core/tz.hpp"
#include "core/report.hpp"
#include "core/profiling/zone.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include "gl/impl/backend/vk2/extensions.hpp"
#include <algorithm>
#include <utility>
#include <cstring>

namespace tz::gl::vk2
{
	VulkanInstance* inst = nullptr;

	void initialise(tz::GameInfo game_info)
	{
		TZ_PROFZONE("Vulkan Backend - Backend Initialise", TZ_PROFCOL_RED);
		tz_assert(inst == nullptr, "Vulkan Backend already initialised");
		inst = new VulkanInstance
		{{
			.game_info = game_info,
#if TZ_DEBUG
			.extensions = {InstanceExtension::DebugMessenger},
#endif
			.window = &tz::window()
		}};
		tz_report("Vulkan v%u.%u Initialised", vulkan_version.major, vulkan_version.minor);
	}

	void terminate()
	{
		tz_assert(inst != nullptr, "Not initialised");
		delete inst;
		inst = nullptr;
		tz_report("Vulkan v%u.%u Terminated", vulkan_version.major, vulkan_version.minor);
	}

	const VulkanInstance& get()
	{
		tz_assert(inst != nullptr, "Not initialised");
		return *inst;
	}

	inline VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback
	(
		[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
		[[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		[[maybe_unused]] void* user_data
	)
	{
		tz_error("[Vulkan Debug Callback]: %s\n", callback_data->pMessage);
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
		create.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		create.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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
				tz_error("Failed to create DebugMessenger because we ran out of memory.");
			break;
			default:
				tz_error("Failed to create DebugMessenger, but couldn't find out how. This unrecognised error code is undocumented by the Vulkan API, so it's probably something very weird. Please submit a bug report!");
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

	WindowSurface::WindowSurface(const VulkanInstance& instance, const tz::Window& window):
	surface(VK_NULL_HANDLE),
	instance(&instance),
	window(&window)
	{
		tz_assert(!this->window->is_null(), "Cannot create WindowSurface off of a null window. GLFW has likely failed. Please submit a bug report.");

		VkResult res = glfwCreateWindowSurface(this->instance->native(), this->window->get_middleware_handle(), nullptr, &this->surface);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				tz_error("Failed to find either a Vulkan loader or a minimally functional ICD (installable client driver), cannot create Window Surface. If you're an end-user, please ensure your drivers are upto-date -- Note that while this is almost certainly *not* a bug, this is a fatal error and the application must crash.");
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz_error("The provided VulkanInstance does not support window surface creation. Please submit a bug report.");
			break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				tz_error("Window was not created to be used for Vulkan (client api hint wasn't set to GLFW_NO_API). Please submit a bug report.");
			break;
			default:
				tz_error("Window surface creation failed, but for an unknown reason. Please ensure your computer meets the minimum requirements for this program. If you are absolutely sure that your machine is valid, please submit a bug report.");
			break;
		}
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
		tz_assert(this->instance != nullptr, "WindowSurface had null instance");
		return *this->instance;
	}

	const tz::Window& WindowSurface::get_window() const
	{
		tz_assert(this->window != nullptr && !this->window->is_null(), "WindowSurface had nullptr or null tz::Window. Please submit a bug report.");
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
		[[maybe_unused]] const char* enabled_layers = "VK_LAYER_KHRONOS_validation";

		// Extensions (Specified from VulkanInstanceInfo + GLFW)
		util::VkExtensionList extension_natives;
		// Use what we asked for, plus everything GLFW wants.
		std::uint32_t glfw_extension_count;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		extension_natives.resize(info.extensions.count() + glfw_extension_count);
		std::transform(info.extensions.begin(), info.extensions.end(), extension_natives.begin(), [](InstanceExtension ext){return util::to_vk_extension(ext);});
		for(std::size_t i = 0; std::cmp_less(i, glfw_extension_count); i++)
		{
			std::size_t idx = i + info.extensions.count();
			extension_natives[idx] = glfw_extensions[i];
		}

		void* inst_create_pnext = nullptr;
		// Debug Messenger
		const bool create_debug_validation = info.extensions.contains(InstanceExtension::DebugMessenger) && TZ_DEBUG;
		tz_assert(info.window != nullptr && !info.window->is_null(), "Null window provided. Please submit a bug report.");
		VkDebugUtilsMessengerCreateInfoEXT debug_validation_create;
		if(create_debug_validation)
		{
			debug_validation_create = make_debug_messenger_info();
			inst_create_pnext = &debug_validation_create;
		}

		VkInstanceCreateInfo create
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = inst_create_pnext,
			.flags = 0,
			.pApplicationInfo = &app_info,
			#if TZ_DEBUG
			.enabledLayerCount = 1,
			.ppEnabledLayerNames = &enabled_layers,
			#else
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			#endif
			.enabledExtensionCount = static_cast<std::uint32_t>(extension_natives.length()),
			.ppEnabledExtensionNames = extension_natives.data()
		};
		VkResult res = vkCreateInstance(&create, nullptr, &this->instance);
		switch(res)
		{
			case VK_SUCCESS:

			break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				tz_error("Failed to create VulkanInstance because we ran out of host memory (RAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				tz_error("Failed to create VulkanInstance because we ran out of device memory (VRAM). Please ensure that your system meets the minimum requirements.");
			break;
			case VK_ERROR_INITIALIZATION_FAILED:
				tz_error("Failed to create VulkanInstance due to an implementation-specific reason that has not made itself clear. Ensure that your machine supports Vulkan 1.2");
			break;
			case VK_ERROR_LAYER_NOT_PRESENT:
				tz_error("Failed to create VulkanInstance because one or more of the provided %zu layers aren't available on this machine. Please submit a bug report.", create.enabledLayerCount);
			break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				tz_error("Failed to create VulkanInstance because one or more of the provided %zu extensions is not supported on this machine. Please submit a bug report.", create.enabledExtensionCount);
			break;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				tz_error("Failed to create VulkanInstance because Vulkan %u.%u is not supported on this machine. You cannot run this build/version of Topaz on this machine unless you update your drivers such that support is achieved. Alternatively have you tried the OpenGL build?", vulkan_version.major, vulkan_version.minor);
			break;
			default:
				tz_error("Failed to create VulkanInstance but cannot determine why. Please submit a bug report.");
			break;
		}
		if(create_debug_validation)
		{
			this->maybe_debug_messenger = VulkanDebugMessenger{*this};
		}
		this->maybe_window_surface = WindowSurface{*this, *info.window};
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

	const WindowSurface& VulkanInstance::get_surface() const
	{
		tz_assert(this->has_surface(), "VulkanInstance did not have attached surface. Please submit a bug report.");
		return this->maybe_window_surface.value();
	}

	bool VulkanInstance::operator==(const VulkanInstance& rhs) const
	{
		return this->instance == rhs.instance;
	}

	VulkanInstance::NativeType VulkanInstance::native() const
	{
		return this->instance;
	}

}

#endif // TZ_VULKAN
