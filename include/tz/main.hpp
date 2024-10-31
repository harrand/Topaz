#ifndef TZMAIN_HPP
#ifdef _WIN32
#if TOPAZ_SHIPPING
	#define NOMINMAX
	#include <Windows.h>
	#define tz_main() WINAPI wWinMain([[maybe_unused]] HINSTANCE, [[maybe_unused]] HINSTANCE,[[maybe_unused]] LPWSTR, [[maybe_unused]] _In_ int)
#else
	#define tz_main() main()
#endif
#endif
#endif // TZMAIN_HPP 