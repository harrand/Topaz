#ifndef TZ_DATA_VERSION_HPP
#define TZ_DATA_VERSION_HPP
#include <string>
#include <cstdio>
#include <array>

namespace tz
{
	enum class version_type
	{
		normal,
		alpha,
		beta,
		rc
	};
	struct version
	{
		unsigned int major, minor, patch;
		version_type type;


		std::string to_string() const
		{
			const char* suffix;
			switch(this->type)
			{
				case version_type::alpha:
					suffix = " alpha";
				break;
				case version_type::beta:
					suffix = " beta";
				break;
				case version_type::rc:
					suffix = " release candidate";
				break;
				default:
					suffix = "";
				break;
			}
			return std::to_string(this->major) + "." + std::to_string(this->minor) + "." + std::to_string(this->patch) + std::string(suffix);
		}

		static version from_string(std::string_view sv)
		{
			version ret;
			std::sscanf(sv.data(), "%u.%u.%u", &ret.major, &ret.minor, &ret.patch);
			return ret;
		}

		static version from_binary_string(unsigned int ver_numeral)
		{
			return
			{
				.major = (ver_numeral >> 12) & 0xF,
				.minor = (ver_numeral >> 8) & 0xF,
				.patch = (ver_numeral >> 4) & 0xF,
				.type = static_cast<version_type>(ver_numeral & 0xF)
			};
		}

		bool operator==(const version& rhs) const = default;
	};

	version get_version();
}

#endif // TZ_DATA_VERSION_HPP
