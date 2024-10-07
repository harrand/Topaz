#ifdef _WIN32
#include "tz/os/window.hpp"
#include "tz/os/input.hpp"
#include "tz/os/file.hpp"
#include "tz/topaz.hpp"
#include <windows.h>
#include <dwmapi.h>

namespace tz::os
{
	LRESULT CALLBACK impl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	std::pair<DWORD, DWORD> impl_get_window_dims();

	constexpr char wndclass_name[] = "Topaz Window";
	auto hinst = GetModuleHandle(nullptr);
	bool initialised = false;
	bool window_open = false;
	char_type_callback kb_callback = nullptr;
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
		if(wnd != nullptr)
		{
			close_window();
		}
		UnregisterClassA(wndclass_name, hinst);
		initialised = false;
	}

	tz::error_code open_window(window_info winfo)
	{
		ERROR_UNLESS_INITIALISED;
		if(wnd != nullptr)
		{
			close_window();
		}

		bool centrered = winfo.flags & window_flags::centered_window;
		wnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, wndclass_name, winfo.name.c_str(), WS_OVERLAPPEDWINDOW, centrered ? CW_USEDEFAULT : winfo.x, centrered ? CW_USEDEFAULT : winfo.y, winfo.width, winfo.height, nullptr, nullptr, hinst, nullptr);
		if(wnd == nullptr)
		{
			return tz::error_code::unknown_error;
		}
		auto ret = tz::error_code::success;
		if(winfo.flags & window_flags::transparent)
		{
			DWM_BLURBEHIND bb =
			{
				.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION,
				.fEnable = TRUE,
				.hRgnBlur = CreateRectRgn(0, 0, -1, -1),
				.fTransitionOnMaximized = 0
			};
			HRESULT res = DwmEnableBlurBehindWindow(wnd, &bb);
			if(!SUCCEEDED(res))
			{
				ret = tz::error_code::partial_success;
			}
		}
		if(!(winfo.flags & window_flags::invisible))
		{
			ShowWindow(wnd, winfo.flags & window_flags::maximised ? SW_SHOWMAXIMIZED : SW_SHOW);
		}
		window_open = true;
		return ret;
	}

	tz::error_code close_window()
	{
		ERROR_UNLESS_INITIALISED;
		if(wnd == nullptr)
		{
			return tz::error_code::precondition_failure;
		}
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
		if(!window_is_open())
		{
			return;
		}
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

	unsigned int window_get_width()
	{
		return impl_get_window_dims().first;
	}

	unsigned int window_get_height()
	{
		return impl_get_window_dims().second;
	}

	tz::error_code install_char_typed_callback(char_type_callback callback)
	{
		if(wnd == nullptr)
		{
			return tz::error_code::precondition_failure;
		}
		kb_callback = callback;
		return tz::error_code::success;
	}

	std::expected<std::string, tz::error_code> read_file(std::filesystem::path path)
	{
		HANDLE file = CreateFileA(path.string().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if(file == INVALID_HANDLE_VALUE)
		{
			// file does not exist.
			UNERR(tz::error_code::precondition_failure, "failed to open file \"{}\"", path.string());
		}
		LARGE_INTEGER file_size;
		GetFileSizeEx(file, &file_size);
		std::string ret;
		ret.resize(file_size.QuadPart, '\0');
		DWORD real_byte_count;
		BOOL read_status = ReadFile(file, ret.data(), ret.size(), &real_byte_count, nullptr);
		ret.resize(real_byte_count);
		if(!read_status && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			UNERR(tz::error_code::engine_bug, "windows read file logic is invalid. buffer was too small even when i specifically asked for the size correctly.");
		}
		if(read_status)
		{
			return ret;
		}
		UNERR(tz::error_code::unknown_error, "undocumented windows error occurred when trying to read file");
	}

	// IMPL

	std::pair<DWORD, DWORD> impl_get_window_dims()
	{
		RECT rect;
		if(wnd != nullptr && GetWindowRect(wnd, &rect))
		{
			return {rect.right - rect.left, rect.bottom - rect.top};
		}
		return {0, 0};
	}

	LRESULT CALLBACK impl_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch(msg)
		{
			case WM_CLOSE:
				window_open = false;
			break;
			case WM_CHAR:
				if(kb_callback != nullptr)
				{
					if(wparam == '\r')
					{
						wparam = '\n';
					}
					kb_callback(wparam);
				}
			break;
		}
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}
}

#endif // _WIN32