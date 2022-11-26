#include "hdk/debug.hpp"
#include <string_view>
#include <cstdint>
#include <iterator>

namespace tz
{
	EngineInfo info()
	{
		EngineInfo inf{};
		#if TZ_VULKAN
			inf.renderer = EngineInfo::RenderAPI::Vulkan;
		#elif TZ_OGL
			inf.renderer = EngineInfo::RenderAPI::OpenGL;
		#else
			static_assert(false, "tz::info(): No RenderAPI has been set. Can't build.");
		#endif

		#if HDK_DEBUG
			inf.build = EngineInfo::BuildConfig::Debug;
		#elif HDK_PROFILE
			inf.build = EngineInfo::BuildConfig::Profile;
		#else
			inf.build = EngineInfo::BuildConfig::Release;
		#endif
		
		inf.version = hdk::version::from_binary_string(TZ_VERSION);
		return inf;
	}
}
