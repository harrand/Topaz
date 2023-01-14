#ifdef __linux__
#include "tz/wsi/impl/linux/monitor.hpp"
#include "tz/wsi/impl/linux/wsi_linux.hpp"
#include <X11/extensions/Xrandr.h>

namespace tz::wsi::impl
{
	std::vector<tz::wsi::monitor> get_monitors_linux()
	{
		auto x11d = impl::x11_display().display;
		std::vector<tz::wsi::monitor> ret;
		XRRScreenResources* resources = XRRGetScreenResourcesCurrent(x11d, DefaultRootWindow(x11d));
		for(int i = 0; i < resources->noutput; i++)
		{
			XRROutputInfo* out_info = XRRGetOutputInfo(x11d, resources, resources->outputs[i]);
			XRRCrtcInfo* crtc = XRRGetCrtcInfo(x11d, resources, out_info->crtc);
			ret.push_back
			({
				.name = std::string(out_info->name),
				.dimensions = {crtc->width, crtc->height}
			});
			if(resources->outputs[i] == XRRGetOutputPrimary(x11d, DefaultRootWindow(x11d)))
			{
				// This is the primary monitor. Make this the first element.
				std::swap(ret.front(), ret.back());
			}
		}
		XRRFreeScreenResources(resources);
		return ret;
	}
}

#endif // __linux__
