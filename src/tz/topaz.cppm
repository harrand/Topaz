module;
#include <string>
#include <format>
export module topaz;

bool initialised = false;
export namespace tz
{
	//	  				Initialise & Terminate					//
	//----------------------------------------------------------//
	void initialise()
	{
		initialised = true;
	}	

	void terminate()
	{
		initialised = false;
	}

	bool is_initialised()
	{
		return initialised;
	}

	//		  				Engine Version						//
	//----------------------------------------------------------//

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
			return std::format("{}.{}.{}{}", this->major, this->minor, this->patch, suffix);
		}
	};

	constexpr version get_engine_version()
	{
		return
		{
			.major = (TZ_VERSION >> 12) & 0xF,
			.minor = (TZ_VERSION >> 8) & 0xF,
			.patch = (TZ_VERSION >> 4) & 0xF,
			.type = static_cast<version_type>(TZ_VERSION & 0xF)
		};
	}
}
