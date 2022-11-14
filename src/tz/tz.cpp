#include "tz/tz.hpp"
#include "hdk/hdk.hpp"

namespace tz
{
	void initialise()
	{
		hdk::initialise();
	}

	void terminate()
	{
		hdk::terminate();
	}
}
