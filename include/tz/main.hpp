#ifndef TZMAIN_HPP
#ifdef _WIN32
	#define NOMINMAX
	#include <Windows.h>
	#define tz_main() WINAPI wWinMain([[maybe_unused]] HINSTANCE, [[maybe_unused]] HINSTANCE,[[maybe_unused]] LPWSTR, [[maybe_unused]] _In_ int)
#endif
#endif // TZMAIN_HPP 