#include "core/assert.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

int main()
{
	tz::GameInfo game{"vk_initialise_test", tz::Version{1, 0, 0}, tz::info()};
	{
		tz::gl::vk2::VulkanInfo vk{game};

		{
			using namespace tz::gl::vk2;
			VkApplicationInfo vk_info = vk.native();
			tz_assert(vk_info.apiVersion == util::tz_to_vk_version(vulkan_version), "TODO: Improve message");
			// Ensure no magic extensions are sneaked in. When we do things like this, we have to specify everything.
			tz_assert(vk.get_extensions().empty(), "Default VulkanInfo based on GameInfo somehow has extensions specified (we don't want this done automatically, we should do this manually)");

			ExtensionList exts;
			VulkanInfo vk2{game, exts};
			VulkanInstance inst{vk2};	
			tz_assert(inst.get_info() == vk2, "VulkanInstance::get_info() != Info passed into ctor");
			//WindowSurface surface{inst, tz::window()};
		}
	}
	return 0;
}
