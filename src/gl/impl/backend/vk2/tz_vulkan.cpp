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

	const ExtensionList& VulkanInfo::get_extensions() const
	{
		return this->extensions;
	}

	VulkanInstance::VulkanInstance(VulkanInfo info):
	info(info),
	info_native(this->info.native()),
	inst_info()
	{
		tz_assert(tz::is_initialised(), "VulkanInstance constructed before tz::initialise()");
		// Basic Application Info
		this->inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		this->inst_info.pApplicationInfo = &this->info_native;

		// Extensions
		this->inst_info.enabledExtensionCount = this->info.get_extensions().length();
		this->inst_info.ppEnabledExtensionNames = this->info.get_extensions().data();
		
		// Validation Layers
	}
}

#endif // TZ_VULKAN
