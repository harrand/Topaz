#include "topaz.hpp"

namespace tz
{
	void initialise(appinfo info)
	{
		gpu::initialise(info);
	}

	void terminate()
	{
		gpu::terminate();
	}
}