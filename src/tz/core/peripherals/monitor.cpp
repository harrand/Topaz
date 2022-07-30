#include "tz/core/peripherals/monitor.hpp"
#include "tz/core/profiling/zone.hpp"
#include "GLFW/glfw3.h"
#include "tz/core/tz.hpp"

namespace tz
{
	MonitorInfo from_glfw_monitor(GLFWmonitor* monitor, unsigned int idx)
	{
		int px, py;
		glfwGetMonitorPhysicalSize(monitor, &px, &py);
		float cx, cy;
		glfwGetMonitorContentScale(monitor, &cx, &cy);
		const GLFWvidmode* glfw_vm = glfwGetVideoMode(monitor);
		return
		{
			.monitor_index = idx,
			.physical_dimensions_millimetres = {static_cast<unsigned int>(px), static_cast<unsigned int>(py)},
			.screen_dimensions = {static_cast<unsigned int>(glfw_vm->width), static_cast<unsigned int>(glfw_vm->height)},
			.content_scale = {cx, cy},
			.name = glfwGetMonitorName(monitor)
		};
	}

	namespace detail::peripherals::monitor
	{
		tz::BasicList<MonitorInfo> all_monitors;
		bool initialised = false;

		void initialise()
		{
			TZ_PROFZONE("Monitor Peripherals Initialise", TZ_PROFCOL_BLUE);
			// Assume glfw has already been initialised. We don't yet have a way to check it.
			int mon_count;
			GLFWmonitor** mons = glfwGetMonitors(&mon_count);
			all_monitors.resize(mon_count);

			for(unsigned int i = 0; std::cmp_less(i, mon_count); i++)
			{
				all_monitors[i] = tz::from_glfw_monitor(mons[i], i);
			}
			initialised = true;
		}

		void terminate()
		{
			TZ_PROFZONE("Monitor Peripherals Terminate", TZ_PROFCOL_BLUE);
			int last_mon_count;
			glfwGetMonitors(&last_mon_count);
			tz_assert(std::cmp_equal(last_mon_count, all_monitors.length()), "Number of connected monitors changed throughout runtime. Topaz does not yet support dynamically adding monitors.");
			all_monitors.clear();
			initialised = false;
		}
	}

	MonitorInfo get_default_monitor()
	{
		tz_assert(!get_monitors().empty(), "No monitors detected. Cannot retrieve default monitor.");
		return get_monitors().front();
	}

	tz::BasicList<MonitorInfo> get_monitors()
	{
		tz_assert(detail::peripherals::monitor::initialised, "Monitors submodule not initialised. You forgot to invoke `tz::initialise`?");
		return detail::peripherals::monitor::all_monitors;
	}
}
