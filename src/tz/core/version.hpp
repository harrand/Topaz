#ifndef TOPAZ_CORE_VERSION_HPP
#define TOPAZ_CORE_VERSION_HPP
#include <string>

namespace tz
{
	struct Version
	{
		unsigned int major;
		unsigned int minor;
		unsigned int patch;

		auto operator<=>(const Version& rhs) const = default;
	};

	constexpr Version parse_version(const char* version_string);
	std::string stringify_version(Version version);
}
#include "tz/core/version.inl"

#endif // TOPAZ_CORE_VERSION_HPP
