#if TZ_VULKAN
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#include "GLFW/glfw3.h"
#include <algorithm>
#include <utility>
#include <cstring>

namespace tz::gl::vk2
{
	VulkanInfo::VulkanInfo(tz::GameInfo game_info, ExtensionList extensions):
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

	const ExtensionList& VulkanInfo::get_extensions() const
	{
		return this->extensions;
	}

	bool extension_supported(VkExtension extension)
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

	VulkanInstance::VulkanInstance(VulkanInfo info):
	info(info),
	info_native(this->info.native()),
	extensions(),
	inst_info()
	{
		//tz_assert(tz::is_initialised(), "VulkanInstance constructed before tz::initialise()");
		// Basic Application Info
		this->inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		this->inst_info.pApplicationInfo = &this->info_native;

		// Extensions
		// TODO: Only conditionally add these extra glfw extensions.
		{
			std::uint32_t glfw_extension_count;
			const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
			for(int i = 0; i < std::cmp_less(i, glfw_extension_count); i++)
			{
				this->extensions.add(glfw_extensions[i]);
				tz_assert(extension_supported(glfw_extensions[i]), "The GLFW extension \"%s\" is not supported by the machine. Windowed applictions are not possible in this state.", glfw_extensions[i]);
			}
		}
		for(Extension extension : this->info.get_extensions())
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
	}

	const VulkanInfo& VulkanInstance::get_info() const
	{
		return this->info;
	}

	VkInstance VulkanInstance::native() const
	{
		return this->instance;
	}

	WindowSurface::WindowSurface(const VulkanInstance& instance, const tz::Window& window):
	surface(VK_NULL_HANDLE),
	instance(&instance)
	{
		VkResult res = glfwCreateWindowSurface(this->instance->native(), window.get_middleware_handle(), nullptr, &this->surface);
		switch(res)
		{
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

	WindowSurface::~WindowSurface()
	{
		vkDestroySurfaceKHR(this->instance->native(), this->surface, nullptr);
	}

}

#endif // TZ_VULKAN
