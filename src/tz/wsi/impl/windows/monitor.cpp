#ifdef _WIN32
#include "tz/wsi/impl/windows/monitor.hpp"
#include "tz/wsi/impl/windows/detail/winapi.hpp"

namespace tz::wsi::impl
{
	BOOL CALLBACK mon_enum_proc(HMONITOR hmon, [[maybe_unused]] HDC hdcmon, [[maybe_unused]] LPRECT lprcmon, LPARAM dwdata)
	{
		auto mons = reinterpret_cast<std::vector<tz::wsi::monitor>*>(dwdata);
		MONITORINFOEXA minfo;
		minfo.cbSize = sizeof(MONITORINFOEXA);
		GetMonitorInfo(hmon, &minfo);
		tz::vec2i dims{minfo.rcMonitor.right - minfo.rcMonitor.left, minfo.rcMonitor.bottom - minfo.rcMonitor.top};
		mons->push_back
		({
			.name = minfo.szDevice,
			.dimensions = static_cast<tz::vec2ui>(dims)
		});
		if(hmon == MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY))
		{
			std::swap(mons->front(), mons->back());
		}
		return TRUE;
	}
	std::vector<tz::wsi::monitor> get_monitors_windows()
	{
		std::vector<tz::wsi::monitor> mons;
		EnumDisplayMonitors(nullptr, nullptr, mon_enum_proc, reinterpret_cast<LPARAM>(&mons));
		return mons;
	}
}

#endif // _WIN32
