#include "tz/topaz.hpp"

namespace tz
{
	void initialise(appinfo info)
	{
		detail::job_system_initialise();
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