#include "core/assert.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

void custom_instance()
{
	using namespace tz::gl::vk2;
	VulkanInstance custom
	{{
		.game_info = {"Custom Vulkan", tz::Version{1, 0, 0}, tz::info()},
		.app_type = tz::ApplicationType::Headless
	}};
}

int main()
{
	tz::GameInfo game{"vk_initialise_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	{
		custom_instance();
	}
	tz::terminate();
	return 0;
}
