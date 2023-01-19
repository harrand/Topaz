#include "tz/core/debug.hpp"
#include <string_view>
#include <cstdint>
#include <iterator>

namespace tz
{
	engine_info info()
	{
		engine_info inf{};
		#if TZ_VULKAN
			inf.renderer = engine_info::render_api::vulkan;
		#elif TZ_OGL
			inf.renderer = engine_info::render_api::opengl;
		#else
			static_assert(false, "tz::info(): No render_api has been set. Can't build.");
		#endif

		#if TZ_DEBUG
			inf.build = engine_info::build_config::debug;
		#elif TZ_PROFILE
			inf.build = engine_info::build_config::profile;
		#else
			inf.build = engine_info::build_config::release;
		#endif
		
		inf.version = tz::version::from_binary_string(TZ_VERSION);
		return inf;
	}
}
