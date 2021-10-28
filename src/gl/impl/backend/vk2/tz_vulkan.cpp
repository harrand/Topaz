#include "gl/impl/backend/vk2/extensions.hpp"
#if TZ_VULKAN
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <utility>
#include <cstring>

namespace tz::gl::vk2
{
	VulkanInstance* inst = nullptr;
	WindowSurface* surf = nullptr;

	void initialise(tz::GameInfo game_info, tz::ApplicationType app_type)
	{
		VulkanInfo vk_info{game_info, InstanceExtensionList{InstanceExtension::DebugMessenger}};
		inst = new VulkanInstance{vk_info, app_type};
		surf = new WindowSurface{*inst, tz::window()};
	}

	void initialise_headless(tz::GameInfo game_info, tz::ApplicationType app_type)
	{
		tz_assert(inst == nullptr, "Double initialise");
		VulkanInfo vk_info{game_info, InstanceExtensionList{InstanceExtension::DebugMessenger}};
		inst = new VulkanInstance{vk_info, app_type};
	}

	void terminate()
	{
		tz_assert(inst != nullptr, "Not initialised");
		if(surf != nullptr)
		{
			delete surf;
			surf = nullptr;
		}
		delete inst;
		inst = nullptr;
	}

	const VulkanInstance& get()
	{
		tz_assert(inst != nullptr, "Not initialised");
		return *inst;
	}

	bool is_headless()
	{
		tz_assert(inst != nullptr, "Not initialised");
		return surf != nullptr;
	}

	const WindowSurface& get_window_surface()
	{
		tz_assert(is_headless(), "Cannot retrieve window surface in headless applictions.");
		return *surf;
	}

	VulkanInfo::VulkanInfo(tz::GameInfo game_info, InstanceExtensionList extensions):
	game_info(game_info),
	engine_name(this->game_info.engine.to_string()),
	extensions(extensions)
	{}

	VkApplicationInfo VulkanInfo::native() const
	{
		VkApplicationInfo info{};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pApplicationName = this->game_info.name;
		info.applicationVersion = util::tz_to_vk_version(this->game_info.version);
		
		info.pEngineName = this->engine_name.c_str();
		info.engineVersion = util::tz_to_vk_version(this->game_info.engine.version);
		info.apiVersion = util::tz_to_vk_version(vulkan_version);
		return info;
	}

	const InstanceExtensionList& VulkanInfo::get_extensions() const
	{
		return this->extensions;
	}

	bool VulkanInfo::has_debug_validation() const
	{
		return this->extensions.contains(InstanceExtension::DebugMessenger) && TZ_DEBUG;
	}

	inline VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
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
		VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator
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
		tz_assert(instance.get_info().get_extensions().contains(InstanceExtension::DebugMessenger), "VulkanInstance provided does not support %s, but is trying to initialie a VulkanDebugMessenger. Please submit a bug report.", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		
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

	bool extension_supported(util::VkExtension extension)
	{
		std::uint32_t ext_count;
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
		std::vector<VkExtensionProperties> exts;
		exts.resize(ext_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, exts.data());
		return std::any_of(exts.begin(), exts.end(), [extension](VkExtensionProperties props)->bool
		{
			return std::strcmp(props.extensionName, extension) == 0;
		});
	}

	VulkanInstance::VulkanInstance(VulkanInfo info, tz::ApplicationType type):
	info(info),
	app_type(type),
	info_native(this->info.native()),
	extensions(),
	inst_info(),
	debug_messenger(std::nullopt)
	{
		tz_assert(tz::is_initialised(), "VulkanInstance constructed before tz::initialise()");
		// Basic Application Info
		this->inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		this->inst_info.pApplicationInfo = &this->info_native;

		// Extensions
		if(this->app_type != tz::ApplicationType::Headless)
		{
			std::uint32_t glfw_extension_count;
			const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
			for(int i = 0; std::cmp_less(i, glfw_extension_count); i++)
			{
				this->extensions.add(glfw_extensions[i]);
				tz_assert(extension_supported(glfw_extensions[i]), "The GLFW extension \"%s\" is not supported by the machine. Windowed applictions are not possible in this state.", glfw_extensions[i]);
			}
		}
		for(InstanceExtension extension : this->info.get_extensions())
		{
			this->extensions.add(util::to_vk_extension(extension));
		}

		this->inst_info.enabledExtensionCount = this->extensions.length();
		this->inst_info.ppEnabledExtensionNames = this->extensions.data();	

		// Validation Layers
		// So on TZ_DEBUG we want to enable all validation layers.
		// Otherwise no, no validation layers at all.
		#if TZ_DEBUG
			using ValidationLayer = const char*;
			using ValidationLayerList = tz::BasicList<ValidationLayer>;
			// So we need to pass in a list of layer names, but we enumerate layers via VkLayerProperties, which contains, among other things, the layer name.
			// So we retrieve the list of VkLayerProperties and then fill in their names.
			std::uint32_t layer_count;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			std::vector<VkLayerProperties> layer_props(layer_count);
			ValidationLayerList enabled_layers;
			enabled_layers.resize(layer_count);
			enabled_layers.resize(layer_count);
			vkEnumerateInstanceLayerProperties(&layer_count, layer_props.data());
			// Retrieve each name
			std::transform(layer_props.begin(), layer_props.end(), enabled_layers.begin(), [](const VkLayerProperties& prop){return prop.layerName;});

			// Now pass it to the create info.
			this->inst_info.enabledLayerCount = layer_count;
			this->inst_info.ppEnabledLayerNames = enabled_layers.data();
		#else // !TZ_DEBUG, AKA Release
			this->inst_info.enabledLayerCount = 0;
			this->inst_info.ppEnabledLayerNames = nullptr;
		#endif // TZ_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debug_create_temp;
		if(this->info.has_debug_validation())
		{
			// Debug Messenger isn't created until after the instance, so we don't get coverage while creating the instance. We pass a temporary Debug Messenger into inst_info.pNext to provide coverage here.
			debug_create_temp = make_debug_messenger_info();
			inst_info.pNext = &debug_create_temp;
		}

		VkResult res = vkCreateInstance(&this->inst_info, nullptr, &this->instance);
		if(res != VK_SUCCESS)
		{
			if(res == VK_ERROR_LAYER_NOT_PRESENT)
			{
				tz_error("Instance creation failed. One or more of the provided %zu layers don't exist", this->inst_info.enabledLayerCount);
			}
			else if(res == VK_ERROR_EXTENSION_NOT_PRESENT)
			{
				tz_error("Instance creation failed. One or more of the provided %zu extensions is not supported on this machine.", this->inst_info.enabledExtensionCount);
			}
			else
			{
				tz_error("Instance creation failed, but couldn't determine the reason why...");
			}
		}
		if(this->info.has_debug_validation())
		{
			this->debug_messenger = VulkanDebugMessenger{*this};
		}
	}

	VulkanInstance::~VulkanInstance()
	{
		this->debug_messenger = std::nullopt;	

		if(this->instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(this->instance, nullptr);	
			this->instance = VK_NULL_HANDLE;
		}
	}

	const VulkanInfo& VulkanInstance::get_info() const
	{
		return this->info;
	}

	VkInstance VulkanInstance::native() const
	{
		return this->instance;
	}

	bool VulkanInstance::operator==(const VulkanInstance& rhs) const
	{
		return this->instance == rhs.instance;
	}

	WindowSurface::WindowSurface(const VulkanInstance& instance, const tz::Window& window):
	surface(VK_NULL_HANDLE),
	instance(&instance)
	{
		tz_assert(!window.is_null(), "Cannot create WindowSurface off of a null window. GLFW has likely failed. If you are trying to run a headless application, please submit a bug report.");

		VkResult res = glfwCreateWindowSurface(this->instance->native(), window.get_middleware_handle(), nullptr, &this->surface);
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
	instance(nullptr)
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
		return *this;
	}

	VkSurfaceKHR WindowSurface::native() const
	{
		return this->surface;
	}
	
	const VulkanInstance& WindowSurface::get_instance() const
	{
		tz_assert(this->instance != nullptr, "WindowSurface had null instance");
		return *this->instance;
	}

}

#endif // TZ_VULKAN
