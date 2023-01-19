#include "tz/core/data/version.hpp"

namespace tz
{
	version get_version()
	{
		return version::from_binary_string(TZ_VERSION);
	}
}
