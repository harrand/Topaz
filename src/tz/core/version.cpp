#include "tz/core/version.hpp"
#include <string>

namespace tz
{
		
	std::string stringify_version(Version version)
	{
		std::string res = std::to_string(version.major) + ".";
		res += std::to_string(version.minor) + ".";
		res += std::to_string(version.patch);
		return res;
	}
}