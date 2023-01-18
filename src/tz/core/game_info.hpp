#ifndef TOPAZ_CORE_GAME_INFO_HPP
#define TOPAZ_CORE_GAME_INFO_HPP
#include "tz/core/engine_info.hpp"
#include <cstring>

namespace tz
{    
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
	/**
	 * Contains basic information about the target game/application.
	 * @note This struct is within the Initial Group.
	 */
	struct game_info
	{
		/**
		 * @brief Retrieve a string describing the game_info.
		 * - See @ref engine_info::to_string() for `engine_info`
		 * @return 'APPNAME vX.Y.Z (`engine_info`)'
		 */
		inline std::string to_string() const
		{
			std::string res = this->name;
			res += " v";
			res += this->version.to_string();
			res += " (";
			res += this->engine.to_string();
			res += ")";
			return res;
		}

		bool operator==(const game_info& rhs) const
		{
			return std::strcmp(this->name, rhs.name) == 0 && version == rhs.version && engine == rhs.engine;
		}

		const char* name;
		hdk::version version;
		engine_info engine;
	};
	/**
	 * @}
	 */
}

#endif // TOPAZ_CORE_GAME_INFO_HPP
