#include "tz/wsi/window.hpp"
#include <deque>
#include <optional>

namespace tz::wsi
{
	struct window_manager
	{
		std::deque<std::optional<tz::wsi::window>> windows;
	};

	static window_manager wm;

	window_handle create_window(window_info info)
	{
		std::size_t wid = wm.windows.size();
		wm.windows.emplace_back(info);
		return static_cast<tz::hanval>(wid);
	}

	bool destroy_window(window_handle wh)
	{
		auto wid = static_cast<std::size_t>(static_cast<tz::hanval>(wh));
		if(wm.windows.size() <= wid || wm.windows[wid] == std::nullopt)
		{
			return false;
		}
		wm.windows[wid] = std::nullopt;
		return true;
	}

	window& get_window(window_handle wh)
	{
		auto wid = static_cast<std::size_t>(static_cast<tz::hanval>(wh));
		tz::assert(has_window(wh), "No valid window at handle %zu", wid);
		return wm.windows[wid].value();
	}

	bool has_window(window_handle wh)
	{
		auto wid = static_cast<std::size_t>(static_cast<tz::hanval>(wh));
		return wm.windows.size() > wid && wm.windows[wid].has_value();
	}

	std::size_t window_count()
	{
		return std::count_if(wm.windows.begin(), wm.windows.end(), [](const auto& maybe_wnd){return maybe_wnd.has_value();});
	}
}
