#ifndef TZ_PROFILE_HPP
#define TZ_PROFILE_HPP
#if TZ_PROFILE
#include "tracy/Tracy.hpp"
#include "common/TracySystem.hpp"
#include "client/TracyProfiler.hpp"
#endif
#undef assert

#if TZ_PROFILE
	#define CONCAT_tzPROF(a, b) CONCAT_INNER_tzPROF(a, b)
	#define CONCAT_INNER_tzPROF(a, b) a ## b

	#define TZ_UNIQUE_NAME(base) CONCAT_tzPROF(base, __LINE__)
	#define TZ_PROFZONE(name, colour) ZoneNamedNC(TZ_UNIQUE_NAME(tracy_profvar), name, colour, true)
	#define TZ_THREAD(name) tracy::SetThreadName(name)
	#define TZ_FRAME FrameMark
	#define TZ_FRAME_BEGIN FrameMarkStart("Frame Loop")
	#define TZ_FRAME_END FrameMarkEnd("Frame Loop")
#else
	#define TZ_PROFZONE(name, colour)
	#define TZ_THREAD(name)
	#define TZ_FRAME
	#define TZ_FRAME_BEGIN
	#define TZ_FRAME_END
#endif

#endif // TZ_PROFILE_HPP
