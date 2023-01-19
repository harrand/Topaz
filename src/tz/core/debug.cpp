#include "tz/core/debug.hpp"
#include "debugbreak.h"

namespace tz
{
	void debug_break()
	{
		::debug_break();
	}
}
