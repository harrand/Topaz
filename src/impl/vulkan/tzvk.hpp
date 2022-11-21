#ifndef VULKAN_TZVK_HPP
#define VULKAN_TZVK_HPP
#include "hdk/data/version.hpp"

namespace tz::impl_vk
{
	struct initialise_info_t
	{
		hdk::version app_version;
		const char* app_name;
	};

	void initialise(initialise_info_t info);
	void initialise();
	void terminate();
}

#endif // VULKAN_TZVK_HPP
