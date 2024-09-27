#ifdef _WIN32
#include "tz/os/window.hpp"
#include "tz/topaz.hpp"
#include <windows.h>

namespace tz::os
{
	LRESULT CALLBACK impl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	constexpr char wndclass_name[] = "Topaz Window";
	auto hinst = GetModuleHandle(nullptr);
	WNDCLASSEXA wndclass
	{
		.cbSize = sizeof(WNDCLASSEXA),
		.lpfnWndProc = impl_wndproc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hinst,
		.hIcon = LoadIconA(nullptr, MAKEINTRESOURCEA(32512)),
		.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(32512)),
		.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
		.lpszMenuName = nullptr,
		.lpszClassName = wndclass_name,
		.hIconSm = LoadIconA(nullptr, MAKEINTRESOURCEA(32512)),
	};

	void initialise()
	{
		tz_assert(RegisterClassExA(&wndclass), "Window class registration failed: winapi system error code {}", GetLastError());
	}

	void terminate()
	{
		UnregisterClassA(wndclass_name, hinst);
	}

	window_handle create_window(window_info winfo)
	{
		(void)winfo;
		return tz::nullhand;
	}

	LRESULT CALLBACK impl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}
}

#endif // _WIN32