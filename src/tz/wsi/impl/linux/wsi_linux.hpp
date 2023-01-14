#ifndef TANGLE_IMPL_LINUX_TGE_LINUX_HPP
#define TANGLE_IMPL_LINUX_TGE_LINUX_HPP
#ifdef __linux__
#include <X11/Xlib.h>

namespace tz::wsi::impl
{
	struct x11_display_data
	{
		Display* display = nullptr;
		int screen = 0;
	};
	void initialise_linux();
	void terminate_linux();
	void update_linux();

	x11_display_data& x11_display();
	XEvent* get_current_event();
}

#endif // __linux__
#endif // TANGLE_IMPL_LINUX_TGE_LINUX_HPP
