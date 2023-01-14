#ifdef __linux__
#include "tz/wsi/impl/linux/wsi_linux.hpp"
#include "hdk/debug.hpp"
#include <optional>

namespace tz::wsi::impl
{
	static x11_display_data x11d = {};
	static std::optional<XEvent> evt = std::nullopt;

	void initialise_linux()
	{
		x11d.display = XOpenDisplay(nullptr);
		hdk::assert(x11d.display != nullptr, "XOpenDisplay(nullptr) failed. Is an X-server not available?");
		x11d.screen = DefaultScreen(x11d.display);
		hdk::report("Initialised for Linux!");
	}

	void terminate_linux()
	{
		XCloseDisplay(x11d.display);
		x11d.display = nullptr;
		hdk::report("Terminated on Linux!");
	}

	void update_linux()
	{
		evt = XEvent{};
		XNextEvent(x11d.display, &evt.value());
	}

	x11_display_data& x11_display()
	{
		return x11d;
	}

	XEvent* get_current_event()
	{
		if(evt.has_value())
		{
			return &evt.value();
		}
		return nullptr;
	}
}

#endif // __linux__
