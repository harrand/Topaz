#include "tz/topaz.hpp"

namespace tz
{
	void initialise(appinfo info)
	{
		detail::job_system_initialise();
		detail::lua_initialise_all_threads();
		os::initialise();
		gpu::initialise(info);
	}

	void terminate()
	{
		gpu::terminate();
		os::terminate();
		detail::job_system_terminate();
	}
}