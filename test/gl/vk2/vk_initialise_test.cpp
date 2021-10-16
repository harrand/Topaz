#include "core/assert.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

int main()
{
	tz::GameInfo game{"vk_initialise_test", tz::Version{1, 0, 0}, tz::info()};
	tz::gl::vk2::VulkanInfo vk{game};

	{
		VkApplicationInfo vk_info = vk.native();
		tz_assert(vk_info.apiVersion == tz::gl::vk2::util::tz_to_vk_version(tz::gl::vk2::vulkan_version), "TODO: Improve message");
	}
	return 0;
}
