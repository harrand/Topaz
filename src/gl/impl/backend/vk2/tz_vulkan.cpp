#include <algorithm>
#if TZ_VULKAN
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

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

	VulkanInstance::VulkanInstance(VulkanInfo info):
	info(info),
	info_native(this->info.native()),
	extensions(),
	inst_info()
	{
		tz_assert(tz::is_initialised(), "VulkanInstance constructed before tz::initialise()");
		// Basic Application Info
		this->inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		this->inst_info.pApplicationInfo = &this->info_native;

		// Extensions
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
		#else
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
}

#endif // TZ_VULKAN
