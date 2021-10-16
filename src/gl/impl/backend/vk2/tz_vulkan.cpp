#if TZ_VULKAN
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

namespace tz::gl::vk2
{
	VulkanInfo::VulkanInfo(tz::GameInfo game_info):
	game_info(game_info),
	engine_name(this->game_info.engine.to_string()){}
}

#endif // TZ_VULKAN
