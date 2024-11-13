#include "tz/topaz.hpp"
#include "tz/imgui.hpp"

namespace tz
{
	void initialise(appinfo info)
	{
		detail::job_system_initialise();
		detail::lua_initialise_all_threads();
		os::initialise();
		gpu::initialise(info);
		detail::imgui_initialise();
	}

	void terminate()
	{
		detail::imgui_terminate();
		gpu::terminate();
		os::terminate();
		detail::job_system_terminate();
	}
}