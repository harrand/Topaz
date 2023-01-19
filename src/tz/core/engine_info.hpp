#ifndef TOPAZ_CORE_ENGINE_INFO_HPP
#define TOPAZ_CORE_ENGINE_INFO_HPP
#include <string>
#include "tz/core/data/version.hpp"

namespace tz
{
	/**
	 * \addtogroup tz_core Topaz Core Library (tz)
	 * A collection of platform-agnostic core interfaces.
	 * @{
	 */
	/**
	 * Represents information about the current engine used.
	 * @note This struct is within the Initial Group.
	 */
	struct engine_info
	{
		enum class render_api
		{
			vulkan,
			opengl
		};

		enum class build_config
		{
			debug,
			release,
			profile
		};

		/**
		 * @brief Retrieve a string describing the engine.
		 * - `RENDERAPI` is either "Vulkan" or "OpenGL" depending on CMake configuration.
		 * - `BUILDCONFIG` is either "Debug" or "Release" depending on CMake configuration.
		 * @return 'Topaz vX.Y.Z `RENDERAPI` `BUILDCONFIG`' 
		 */
		std::string to_string() const;
		constexpr bool operator==(const engine_info& rhs) const = default;

		render_api renderer;
		build_config build;
		tz::version version;
	};

	/**
	 * @brief Retrieves the engine_info for this specific build of Topaz.
	 * @relates engine_info
	 * 
	 * @return Information about this Topaz build.
	 */
	inline engine_info info();

	/**
	 * @}
	 */
}
#include "tz/core/engine_info.inl"
#endif // TOPAZ_CORE_ENGINE_INFO_HPP
