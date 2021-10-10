#if !TZ_VULKAN
	static_assert(false, "Cannot build vk_init_demo with TZ_VULKAN disabled.");
#endif

#include "core/tz.hpp"

int main()
{
	constexpr tz::EngineInfo eng_info = tz::info();
	constexpr tz::GameInfo vk_init_demo{"vk_init_demo", eng_info.version, eng_info};
	tz::initialise(vk_init_demo, tz::ApplicationType::Headless);
	{
		
	}
	tz::terminate();
	return 0;
}