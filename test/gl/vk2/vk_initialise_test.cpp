#include "core/assert.hpp"
#include "core/tz.hpp"
#include "gl/impl/backend/vk2/tz_vulkan.hpp"

int main()
{
	tz::GameInfo game{"vk_initialise_test", tz::Version{1, 0, 0}, tz::info()};
	tz::initialise(game, tz::ApplicationType::Headless);
	{
		tz::gl::vk2::VulkanInfo vk{game};

		{
			using namespace tz::gl::vk2;
			VkApplicationInfo vk_info = vk.native();
			tz_assert(vk_info.apiVersion == util::tz_to_vk_version(vulkan_version), "TODO: Improve message");
			// Ensure no magic extensions are sneaked in. When we do things like this, we have to specify everything.
			tz_assert(vk.get_extensions().empty(), "Default VulkanInfo based on GameInfo somehow has extensions specified (we don't want this done automatically, we should do this manually)");

			InstanceExtensionList exts;
			VulkanInfo vk2{game, exts};
			tz_assert(!vk2.has_debug_validation(), "VulkanInfo without any extensions wrongly says it has debug validation enabled.");
			VulkanInstance inst{vk2, tz::ApplicationType::Headless};	
			tz_assert(inst.get_info() == vk2, "VulkanInstance::get_info() != Info passed into ctor");
			//WindowSurface surface{inst, tz::window()}; // We're headless, we can't do this!
			InstanceExtensionList exts2{InstanceExtension::DebugMessenger};
			VulkanInfo vk3{game, exts2};
			tz_assert(vk3.has_debug_validation(), "VulkanInfo with ExtensionList containing {Extension::DebugMessenger} wrongly doesn't support debug validation (we're definitely TZ_DEBUG because this is an assert!)");
		}
	}
	tz::terminate();
	return 0;
}
