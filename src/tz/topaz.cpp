#include "tz/topaz.hpp"

namespace tz
{
	void initialise(appinfo info)
	{
		os::initialise();
		gpu::initialise(info);
	}

	void terminate()
	{
		gpu::terminate();
		os::terminate();
	}
}