#ifdef _WIN32
#include "tz/wsi/impl/windows/window.hpp"
#include "tz/wsi/impl/windows/wsi_windows.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include <dwmapi.h>
#include <algorithm>

namespace tz::wsi::impl
{
	window_winapi::window_winapi(window_info info):
	flags(info.window_flags)
	{
		DWORD style = WS_OVERLAPPEDWINDOW;
		if(info.window_flags & window_flag::undecorated)
		{
			style = WS_CAPTION;
		}
		if(info.window_flags & window_flag::bare)
		{
			style = WS_POPUPWINDOW;
		}
		if(info.window_flags & window_flag::noresize && style & WS_THICKFRAME)
		{
			style ^= WS_THICKFRAME;
		}
		if(info.window_flags & window_flag::nomaximise && style & WS_MAXIMIZEBOX)
		{
			style ^= WS_MAXIMIZEBOX;
		}
		if(info.window_flags & window_flag::nominimise && style & WS_MINIMIZEBOX)
		{
			style ^= WS_MINIMIZEBOX;
		}
		this->hwnd = CreateWindowExA(
			0,
			impl::wndclass_name,
			info.title,
			style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			info.dimensions[0], info.dimensions[1],
			nullptr, nullptr,
			GetModuleHandle(nullptr),
			this
		);
		if(info.window_flags & window_flag::transparent)
		{
			DWM_BLURBEHIND bb =
			{
				.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION,
				.fEnable = TRUE,
				.hRgnBlur = CreateRectRgn(0, 0, -1, -1),
				.fTransitionOnMaximized = 0
			};
			HRESULT res = DwmEnableBlurBehindWindow(this->hwnd, &bb);
			tz::assert(SUCCEEDED(res), "Failed to make window transparent");
		}
		tz::assert(this->hwnd != nullptr, "Window creation failed. GetLastError() returns %lu", GetLastError());
		this->hdc = GetDC(this->hwnd);
		if(info.window_flags & window_flag::opengl)
		{
			this->impl_init_opengl();
		}
		this->impl_register_mouseleave();
		if(!(info.window_flags & window_flag::invisible))
		{
			ShowWindow(this->hwnd, SW_SHOW);
		}

		// Empty keyboard state.
		std::fill(this->key_state.keys_down.begin(), this->key_state.keys_down.end(), key::unknown);
		// Empty mouse state.
		std::fill(this->mouse_state.button_state.begin(), this->mouse_state.button_state.end(), mouse_button_state::noclicked);
		this->mouse_state.mouse_position = tz::vec2ui::zero();
	}

//--------------------------------------------------------------------------------------------------

	window_winapi::~window_winapi()
	{
		this->impl_request_close();
	}

//--------------------------------------------------------------------------------------------------

	window_winapi::native window_winapi::get_native() const
	{
		return this->hwnd;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::request_close()
	{
		this->close_requested = true;
	}

//--------------------------------------------------------------------------------------------------

	bool window_winapi::is_close_requested() const
	{
		return this->close_requested;
	}

//--------------------------------------------------------------------------------------------------

	tz::vec2ui window_winapi::get_dimensions() const
	{
		RECT rect;
		[[maybe_unused]] bool ret = GetClientRect(this->hwnd, &rect);
		tz::assert(ret, "Failed to retrieve window dimensions. GetLastError() returns %lu", GetLastError());
		return static_cast<tz::vec2ui>(tz::vec2i{rect.right - rect.left, rect.bottom - rect.top});
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::set_dimensions(tz::vec2ui dims)
	{
		// dims is the dimensions of the client rect, not the window size.
		// we figure out the real window size and use that.
		RECT rc_client, rc_wnd;
		GetClientRect(this->hwnd, &rc_client);
		GetWindowRect(this->hwnd, &rc_wnd);
		tz::vec2i border_dims
		{
			(rc_wnd.right - rc_wnd.left) - rc_client.right,
			(rc_wnd.bottom - rc_wnd.top) - rc_client.bottom
		};
		tz::vec2i real_dims = dims + border_dims;
		SetWindowPos(
			this->hwnd,
			HWND_NOTOPMOST,
			0, 0,
			real_dims[0], real_dims[1],
			SWP_NOMOVE
		);
	}

//--------------------------------------------------------------------------------------------------

	std::string window_winapi::get_title() const
	{
		std::string ret;
		ret.resize(GetWindowTextLengthA(this->hwnd));
		GetWindowTextA(this->hwnd, ret.data(), ret.size());
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::set_title(std::string title)
	{
		[[maybe_unused]] bool ok = SetWindowTextA(this->hwnd, title.data());	
		tz::assert(ok, "Failed to set window title to \"%s\", GetLastError() returned %lu", title.data(), GetLastError());
	}

//--------------------------------------------------------------------------------------------------

	window_flag::flag_bit window_winapi::get_flags() const
	{
		return this->flags;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::update()
	{
		TZ_PROFZONE("window - update", 0xffff0000);
		if(this->close_requested)
		{
			return;
		}
		SwapBuffers(this->hdc);
	}

//--------------------------------------------------------------------------------------------------

	bool window_winapi::make_opengl_context_current()
	{
		tz::assert(this->hdc != nullptr, "Tried to make window opengl context current, but the window was malformed (HDC is invalid)");
		if(this->opengl_rc == nullptr)
		{
			return false;
		}
		return wglMakeCurrent(this->hdc, this->opengl_rc);
	}

//--------------------------------------------------------------------------------------------------

	#if TZ_VULKAN
	VkSurfaceKHR window_winapi::make_vulkan_surface(VkInstance vkinst) const
	{
		VkSurfaceKHR surf;
		VkWin32SurfaceCreateInfoKHR create
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR ,
			.pNext = nullptr,
			.flags = 0,
			.hinstance = (HINSTANCE)GetWindowLongPtrA(this->hwnd, GWLP_HINSTANCE),
			.hwnd = this->hwnd
		};
		auto fn = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(vkinst, "vkCreateWin32SurfaceKHR"));
		tz::assert(fn != nullptr);
		VkResult res = fn(vkinst, &create, nullptr, &surf);
		tz::assert(res == VK_SUCCESS);
		return surf;
	}
	#endif // TZ_VULKAN

//--------------------------------------------------------------------------------------------------

	const keyboard_state& window_winapi::get_keyboard_state() const
	{
		return this->key_state;
	}

//--------------------------------------------------------------------------------------------------

	const mouse_state& window_winapi::get_mouse_state() const
	{
		return this->mouse_state;
	}

//--------------------------------------------------------------------------------------------------

	void* window_winapi::get_user_data() const
	{
		return this->userdata;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::set_user_data(void* udata)
	{
		this->userdata = udata;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::impl_init_opengl()
	{
		tz::wsi::impl::wgl_function_data wgl = tz::wsi::impl::get_wgl_functions();
		tz::assert(wgl != wgl_function_data{}, "Attempting to create modern OpenGL context for window, but WGL function data has not been loaded properly. Did you forget to `tz::wsi::initialise()`?");
		// First, set pixel format.
		{
			int attrib[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_EXT,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_TRANSPARENT_ARB, TRUE,
				WGL_COLOR_BITS_ARB, 24,
				WGL_ALPHA_BITS_ARB, 8,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
				0
			};
			int format;
			UINT formats;
			if(!wgl.wgl_choose_pixel_format_arb(this->hdc, attrib, nullptr, 1, &format, &formats) || formats == 0)
			{
				tz::error("Modern OpenGL does not support the default required pixel format. Your graphics card is probably too old to support modern OpenGL.");
			}
			PIXELFORMATDESCRIPTOR dsc{};
			dsc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			bool ok = DescribePixelFormat(this->hdc, format, sizeof(dsc), &dsc);
			tz::assert(ok, "Failed to describe OpenGL pixel format. No idea why not, I'm afraid.");
			if(!SetPixelFormat(this->hdc, format, &dsc))
			{
				tz::error("Failed to set modern OpenGL pixel format.");
			}
		}

		// Second, create modern opengl context.
		{
			int attrib[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 6,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				#if TZ_DEBUG
					WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
				#endif
				0
			};
			this->opengl_rc = wgl.wgl_create_context_attribs_arb(this->hdc, nullptr, attrib);
			tz::assert(this->opengl_rc != nullptr, "Failed to create modern opengl context (OpenGL 4.5). Perhaps your graphics card does not support this version?");
			[[maybe_unused]] BOOL ok = wglMakeCurrent(this->hdc, this->opengl_rc);
			tz::assert(ok, "Failed to make modern opengl context current.");
			wgl.wgl_swap_interval_ext(0);
		}
	}

//--------------------------------------------------------------------------------------------------

	bool window_winapi::impl_is_opengl() const
	{
		return this->opengl_rc != nullptr;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::impl_request_close()
	{
		this->close_requested = true;
		if(this->hwnd != nullptr)
		{
			DestroyWindow(this->hwnd);
			this->hwnd = nullptr;
		}
	}

//--------------------------------------------------------------------------------------------------

	keyboard_state& window_winapi::impl_mutable_keyboard_state()
	{
		return this->key_state;
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::impl_register_mouseleave()
	{
		TRACKMOUSEEVENT tme
		{
			.cbSize = sizeof(TRACKMOUSEEVENT),
			.dwFlags = TME_LEAVE,
			.hwndTrack = this->hwnd,
			.dwHoverTime = HOVER_DEFAULT
		};
		TrackMouseEvent(&tme);
	}

//--------------------------------------------------------------------------------------------------

	void window_winapi::impl_notify_mouse_enter_window()
	{
		this->mouse_in_window = true;
		this->impl_register_mouseleave();
	}

	void window_winapi::impl_notify_mouse_leave_window()
	{
		this->mouse_in_window = false;
		for(std::size_t i = 0; i < static_cast<std::size_t>(mouse_button::_count); i++)
		{
			this->mouse_state.button_state[i] = mouse_button_state::noclicked;
		}
	}

//--------------------------------------------------------------------------------------------------

	mouse_state& window_winapi::impl_mutable_mouse_state()
	{
		return this->mouse_state;
	}

//--------------------------------------------------------------------------------------------------

	void* get_opengl_proc_address_windows(const char* name)
	{
		// wglGetProcAddress won't work for legacy ogl functions, many of which are still used in modern ogl.
		void *p = (void*)wglGetProcAddress(name);
		// Epic microsoft moment: Doesn't necessarily return nullptr on failure, could return something real small (nice one guys). If so, load from ogl 1.1 dll which is guaranteed to be opengl32.dll.
		if(p == nullptr || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1) )
		{
			HMODULE module = LoadLibraryA("opengl32.dll");
			p = (void *)GetProcAddress(module, name);
		}
		return p;
	}

//--------------------------------------------------------------------------------------------------
}

#endif // WIN32
