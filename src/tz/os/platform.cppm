export module topaz.os;

export namespace tz::os
{
	enum class platform
	{
		windows,
		linux
	};

	constexpr platform get_platform()
	{
		#ifdef _WIN32
			return platform::windows;
		#else
			#ifdef __linux__
				return platform::linux;
			#else
				static_assert(false, "Unknown platform. Either you're building in a dodgy CMake build directory, you're building on an unsupported platform (only Windows and Linux are supported) or something has gone seriously wrong.");
			#endif
		#endif
	}
}
