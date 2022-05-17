#include "core/assert.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

void custom_instance()
{
	using namespace tz::gl::vk2;
	VulkanInstance custom
	{{
		.game_info = {"Custom Vulkan", tz::Version{1, 0, 0}, tz::info()},
		.app_type = tz::ApplicationType::HiddenWindowApplication,
		.window = &tz::window()
	}};
}

int main()
{
	tz::initialise
	({
		.name = "vk_initialise_test",
		.app_type = tz::ApplicationType::HiddenWindowApplication
	});
	{
		custom_instance();
	}
	tz::terminate();
	return 0;
}
