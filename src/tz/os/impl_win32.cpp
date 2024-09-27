#ifdef _WIN32
#include "tz/os/window.hpp"
#include "tz/topaz.hpp"
#include <windows.h>

namespace tz::os
{
	LRESULT CALLBACK impl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	constexpr char wndclass_name[] = "Topaz Window";
	auto hinst = GetModuleHandle(nullptr);
	bool initialised = false;
	bool window_open = false;
	#define ERROR_UNLESS_INITIALISED if(!initialised){return tz::error_code::precondition_failure;}
	WNDCLASSEXA wndclass
	{
		.cbSize = sizeof(WNDCLASSEXA),
		.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
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
	HWND wnd = nullptr;

	// API

	void initialise()
	{
		tz_assert(RegisterClassExA(&wndclass), "Window class registration failed: winapi system error code {}", GetLastError());
		initialised = true;
	}

	void terminate()
	{
		UnregisterClassA(wndclass_name, hinst);
		initialised = false;
	}

	tz::error_code open_window(window_info winfo)
	{
		ERROR_UNLESS_INITIALISED;

		bool centrered = winfo.flags & window_flags::centered_window;
		wnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, wndclass_name, winfo.name.c_str(), WS_OVERLAPPEDWINDOW, centrered ? CW_USEDEFAULT : winfo.x, centrered ? CW_USEDEFAULT : winfo.y, winfo.width, winfo.height, nullptr, nullptr, hinst, nullptr);
		if(wnd == nullptr)
		{
			return tz::error_code::unknown_error;
		}
		ShowWindow(wnd, SW_SHOW);
		window_open = true;
		return tz::error_code::success;
	}

	tz::error_code close_window()
	{
		ERROR_UNLESS_INITIALISED;
		window_open = false;
		auto ret = DestroyWindow(wnd) == 0 ? tz::error_code::unknown_error : tz::error_code::success;
		wnd = nullptr;
		return ret;
	}

	bool window_is_open()
	{
		return window_open;
	}

	void window_update()
	{
		MSG msg{};
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	window_handle get_window_handle()
	{
		if(wnd == nullptr)
		{
			return tz::nullhand;
		}
		return static_cast<tz::hanval>(reinterpret_cast<std::uintptr_t>(wnd));
	}

	// IMPL

	LRESULT CALLBACK impl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch(msg)
		{
			case WM_CLOSE:
				window_open = false;
			break;
		}
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}
}

#endif // _WIN32