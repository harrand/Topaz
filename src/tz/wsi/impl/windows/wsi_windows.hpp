#ifndef TZ_WSI_IMPL_WINDOWS_TGE_HPP
#define TZ_WSI_IMPL_WINDOWS_TGE_HPP
#ifdef _WIN32
#include "tz/wsi/impl/windows/detail/winapi.hpp"
#include <gl/GL.h>
#include <gl/wglext.h>

namespace tz::wsi::impl
{
	constexpr char wndclass_name[] = "Tangle Windowing Library";

	void initialise_windows();
	void terminate_windows();
	void update_windows();
	LRESULT wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	struct wgl_function_data
	{
		PFNWGLCHOOSEPIXELFORMATARBPROC wgl_choose_pixel_format_arb = nullptr;
		PFNWGLCREATECONTEXTATTRIBSARBPROC wgl_create_context_attribs_arb = nullptr;
		PFNWGLSWAPINTERVALEXTPROC wgl_swap_interval_ext = nullptr;
		bool operator==(const wgl_function_data& rhs) const = default;
	};
	wgl_function_data get_wgl_functions();
}

#endif // _WIN32
#endif // TZ_WSI_IMPL_WINDOWS_TGE_HPP
