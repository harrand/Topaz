#ifdef _WIN32
#include "tz/wsi/impl/windows/wsi_windows.hpp"
#include "tz/wsi/impl/windows/window.hpp"
#include "tz/wsi/impl/windows/keyboard.hpp"
#include "hdk/debug.hpp"
#include "hdk/profile.hpp"
#include <algorithm>
#include <string_view>

namespace tz::wsi::impl
{
	constexpr WNDCLASSEXA wndclass_gpuacc
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_OWNDC | CS_DBLCLKS,
		.lpfnWndProc = wndproc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = nullptr,
		.hIcon = nullptr,
		.hCursor = nullptr,
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = wndclass_name,
		.hIconSm = nullptr
	};

	void load_wgl_functions();

//--------------------------------------------------------------------------------------------------

	void initialise_windows()
	{
		auto window_class = wndclass_gpuacc;
		window_class.hInstance = GetModuleHandle(nullptr);
		window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
		RegisterClassExA(&window_class);
		load_wgl_functions();
	}

//--------------------------------------------------------------------------------------------------

	void terminate_windows()
	{
		PostQuitMessage(0);
		UnregisterClassA(wndclass_name, GetModuleHandle(nullptr));
	}

//--------------------------------------------------------------------------------------------------

	void update_windows()
	{
		MSG msg{};
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

//--------------------------------------------------------------------------------------------------

	LRESULT wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		HDK_PROFZONE("Tangle - win32 wndproc", 0xffff0000);
		auto get_window = [hwnd]()
		{
			auto wnd = reinterpret_cast<window_winapi*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			hdk::assert(wnd != nullptr, "window_winapi userdata not setup properly. userdata was nullptr.");
			return wnd;
		};
		switch(msg)
		{
			case WM_CREATE:
			{
				// When created, the window passes a void* for create params, this is guaranteed to be a pointer to the window_winapi. We set it as userdata so we always have access to this pointer.
				auto create = reinterpret_cast<CREATESTRUCT*>(lparam);
				auto wnd = reinterpret_cast<window_winapi*>(create->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
			}
			break;
			case WM_CLOSE:
				get_window()->impl_request_close();	
				return FALSE;
			break;
			case WM_DESTROY:
			break;
			case WM_PAINT:
			{
				HDK_PROFZONE("Tangle - win32 wndproc paint", 0xffff00ff);
				if(!get_window()->impl_is_opengl())
				{
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hwnd, &ps);
					FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
					EndPaint(hwnd, &ps);
				}
			}
			break;
			case WM_KEYDOWN:
			{
				auto& state = get_window()->impl_mutable_keyboard_state();	
				int virtual_keycode = wparam;
				tz::wsi::key k = tz::wsi::impl::win_to_tge_key(virtual_keycode);

				// Retrieve an iter to the first element that is not unknown, and set that value to the key.
				const bool already_pressed = std::any_of(state.keys_down.begin(), state.keys_down.end(), [k](tz::wsi::key key){return key == k;});
				if(k != tz::wsi::key::unknown && !already_pressed)
				{
					auto iter = std::find_if(state.keys_down.begin(), state.keys_down.end(), [](tz::wsi::key key){return key == tz::wsi::key::unknown;});
					hdk::assert(iter != state.keys_down.end(), "There are too many keyboard keys down at once (max = %u)", tz::wsi::max_simultaneous_key_presses);
					*iter = k;
					state.last_key = k;
				}
			}
			break;
			case WM_KEYUP:
			{
				auto& state = get_window()->impl_mutable_keyboard_state();	
				int virtual_keycode = wparam;
				tz::wsi::key k = tz::wsi::impl::win_to_tge_key(virtual_keycode);

				// Retrieve an iter to the first element that is not unknown, and set that value to the key.
				const bool not_pressed = std::none_of(state.keys_down.begin(), state.keys_down.end(), [k](tz::wsi::key key){return key == k;});
				if(k != tz::wsi::key::unknown && !not_pressed)
				{
					auto iter = std::find_if(state.keys_down.begin(), state.keys_down.end(), [k](tz::wsi::key key){return key == k;});
					hdk::assert(iter != state.keys_down.end(), "Key that's meant to have already been pressed (now up) is not considered pressed. Logic error");
					*iter = tz::wsi::key::unknown;
					if(state.last_key == k)
					{
						state.pop_last_key();
					}
				}
			}
			break;
			case WM_MOUSEMOVE:
			{
				auto& state = get_window()->impl_mutable_mouse_state();
				state.mouse_position = {static_cast<unsigned int>(LOWORD(lparam)), static_cast<unsigned int>(HIWORD(lparam))};
			}
			break;
			case WM_LBUTTONDBLCLK:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::left)] = mouse_button_state::double_clicked;
			break;
			case WM_LBUTTONDOWN:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::left)] = mouse_button_state::clicked;
			break;
			case WM_LBUTTONUP:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::left)] = mouse_button_state::noclicked;
			break;

			case WM_RBUTTONDBLCLK:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::right)] = mouse_button_state::double_clicked;
			break;
			case WM_RBUTTONDOWN:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::right)] = mouse_button_state::clicked;
			break;
			case WM_RBUTTONUP:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::right)] = mouse_button_state::noclicked;
			break;

			case WM_MBUTTONDBLCLK:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::middle)] = mouse_button_state::double_clicked;
			break;
			case WM_MBUTTONDOWN:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::middle)] = mouse_button_state::clicked;
			break;
			case WM_MBUTTONUP:
				get_window()->impl_mutable_mouse_state().button_state[static_cast<int>(mouse_button::middle)] = mouse_button_state::noclicked;
			break;
			case WM_MOUSEWHEEL:
				get_window()->impl_mutable_mouse_state().wheel_position += GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
			break;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

//--------------------------------------------------------------------------------------------------

	wgl_function_data wgl_data = {};

	wgl_function_data get_wgl_functions()
	{
		hdk::assert(wgl_data != wgl_function_data{}, "Detected WGL functions have not been loaded properly. This should've been done by `tz::wsi::impl::initialise_windows()`. Have you forgotten to initialise?");
		return wgl_data;
	}

//--------------------------------------------------------------------------------------------------

	void load_wgl_functions()
	{
		hdk::assert(wgl_data == wgl_function_data{}, "Detected WGL functions have already been loaded, but we've been asked to load them a second time. Logic error?");
		HWND dummy = CreateWindowExA(
			0, "STATIC",
			"Dummy Window",
			WS_OVERLAPPED,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			nullptr, nullptr, nullptr, nullptr
		);
		hdk::assert(dummy != nullptr, "Failed to create dummy window. Initialisation has gone pear-shaped, or windows is being extremely dodgy.");
		HDC dc = GetDC(dummy);
		hdk::assert(dc != nullptr, "Failed to retrieve device context for dummy window. Either initialisation has gone pear-shaped, or windows is acting extremely dodgy.");
		PIXELFORMATDESCRIPTOR dsc{};
		dsc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		dsc.nVersion = 1;
		dsc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_COMPOSITION;
		dsc.iPixelType = PFD_TYPE_RGBA;
		dsc.cColorBits = 24;
		int format = ChoosePixelFormat(dc, &dsc);
		hdk::assert(format != 0, "Failed to describe OpenGL pixel format for dummy window. We do this whether or not you use OpenGL, but it seems something has gone very wrong.");
		// we have all this pain because you can only SetPixelFormat once on a window.
		if(!SetPixelFormat(dc, format, &dsc))
		{
			hdk::error("Failed to set pixel format for OpenGL dummy window. We do this whether or not you use OpenGL, but it seems something has gone very wrong.");
		}
		HGLRC rc = wglCreateContext(dc);
		hdk::assert(rc != nullptr, "Failed to create ancient OpenGL context for dummy window.");;
		[[maybe_unused]] bool ok = wglMakeCurrent(dc, rc);
		hdk::assert(ok, "Failed to make ancient OpenGL dummy context current.");
		auto wglGetExtensionsStringARB = reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(reinterpret_cast<void*>(wglGetProcAddress("wglGetExtensionsStringARB")));
		hdk::assert(wglGetExtensionsStringARB != nullptr, "OpenGL does not support WGL_ARB_extensions_string extension! You are most likely using an absolutely ancient GPU, or initialisation has gone extremely pear-shaped.");
		const char* ext = wglGetExtensionsStringARB(dc);
		hdk::assert(ext != nullptr, "Failed to get OpenGL WGL extension string");
		const char* start = ext;
		for(;;)
		{
			while(*ext != 0 && *ext != ' ')
			{
				ext++;
			}

			std::size_t length = ext - start;
			std::string_view cur{start, length};
			if(cur == "WGL_ARB_pixel_format")
			{
				wgl_data.wgl_choose_pixel_format_arb = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(reinterpret_cast<void*>(wglGetProcAddress("wglChoosePixelFormatARB")));
			}
			if(cur == "WGL_ARB_create_context")
			{
				wgl_data.wgl_create_context_attribs_arb = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(reinterpret_cast<void*>(wglGetProcAddress("wglCreateContextAttribsARB")));
			}
			if(cur == "WGL_EXT_swap_control")
			{
				wgl_data.wgl_swap_interval_ext = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(reinterpret_cast<void*>(wglGetProcAddress("wglSwapIntervalEXT")));
			}
			
			if(*ext == '\0')
			{
				break;
			}

			ext++;
			start = ext;
		}
		
		hdk::assert(wgl_data.wgl_choose_pixel_format_arb != nullptr, "Failed to load \"wglChoosePixelFormatARB\" function.");
		hdk::assert(wgl_data.wgl_create_context_attribs_arb != nullptr, "Failed to load \"wglCreateContextAttribsARB\" function.");
		hdk::assert(wgl_data.wgl_swap_interval_ext != nullptr, "Failed to load \"wglSwapIntervalEXT\" function.");
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(rc);
		ReleaseDC(dummy, dc);
		DestroyWindow(dummy);
	}
}

#endif // _WIN32
