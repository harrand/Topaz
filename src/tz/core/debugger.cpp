#include "tz/core/debugger.hpp"
#include "debugbreak.h"

namespace tz
{
	void debug_break()
	{
		::debug_break();
	}
}
