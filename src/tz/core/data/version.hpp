#ifndef TZ_DATA_VERSION_HPP
#define TZ_DATA_VERSION_HPP
#include <string>
#include <cstdio>
#include <array>

namespace tz
{
	/**
	 * Represents the type of the version.
	 */
	enum class version_type
	{
		/// - Normal, or default version. This typically means a full release.
		normal,
		/// - Alpha versions typically mean the product is still early in development and not ready for full release.
		alpha,
		/// - Like alpha, but the product is later in development, and is a more stable candidate for use.
		beta,
		/// - Release Candidate, meaning that this version of the product may be ready for release, aside from a few minor tweaks/fixes.
		rc
	};

	/**
	* @ingroup tz_core
	* Versions consist of a major, minor, patch and a @ref version_type.
	*
	* A version is represented as:
	* major.minor.patch-suffix
	*/
	struct version
	{
		/// Major version.
		unsigned int major;
		/// Minor version.
		unsigned int minor;
		/// Patch version.
		unsigned int patch;
		/// Version type (suffix)
		version_type type;

		/// Retrieve the version as a string. Follows the form major.minor.patch, appended by `-suffix` if the build is not a normal release build.
		std::string to_string() const
		{
			const char* suffix;
			switch(this->type)
			{
				case version_type::alpha:
					suffix = "-alpha";
				break;
				case version_type::beta:
					suffix = "-beta";
				break;
				case version_type::rc:
					suffix = "-rc";
				break;
				default:
					suffix = "";
				break;
			}
			return std::to_string(this->major) + "." + std::to_string(this->minor) + "." + std::to_string(this->patch) + std::string(suffix);
		}

		/// Convert the given string to a version. @note There is currently no support for reading the suffix/version type. The returned object will default to a normal release.
		static version from_string(std::string_view sv)
		{
			version ret;
			std::sscanf(sv.data(), "%u.%u.%u", &ret.major, &ret.minor, &ret.patch);
			return ret;
		}

		/// Convert the given binary-string to a version. For example, 0x0401001 yields v4.1.0-alpha
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

	/**
	* @ingroup tz_core
	* Retrieve the current version of Topaz.
	*
	* Note that the build-config/render-api information is not contained here. For that, you'll want @ref engine_info
	*/
	version get_version();
}

#endif // TZ_DATA_VERSION_HPP
