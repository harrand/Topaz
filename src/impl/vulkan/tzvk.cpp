#include "impl/vulkan/tzvk.hpp"
#include "hdk/debug.hpp"

#include "volk.h"

constexpr std::uint32_t req_vk_version = VK_VERSION_1_3;

namespace tz::impl_vk
{
	namespace detail
	{
		bool initialised = false;
	}

	void initialise()
	{
		hdk::assert(!detail::initialised, "Vulkan backend already initialised!");

		VkResult res = volkInitialize();
		hdk::assert(res == VK_SUCCESS, "`volkInitialize` failed. This means the vulkan loader is not installed on your system.");

		std::uint32_t inst_vk_version = volkGetInstanceVersion();
		hdk::assert(inst_vk_version >= req_vk_version, "Instance-level version does not support at least Vulkan %zu.%zu. It returned %zu.%zu", VK_API_VERSION_MAJOR(req_vk_version), VK_API_VERSION_MINOR(req_vk_version), VK_API_VERSION_MAJOR(inst_vk_version), VK_API_VERSION_MINOR(inst_vk_version));

		detail::initialised = true;
	}

	void terminate()
	{
		hdk::assert(detail::initialised, "Vulkan backend was never initialised!");
		detail::initialised = false;
	}
}
