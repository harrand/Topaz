#ifndef TOPAZ_CORE_ENGINE_INFO_HPP
#define TOPAZ_CORE_ENGINE_INFO_HPP
#include <string>
#include "tz/core/version.hpp"

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
	struct EngineInfo
	{
		enum class RenderAPI
		{
			Vulkan,
			OpenGL
		};

		enum class BuildConfig
		{
			Debug,
			Release,
			Profile
		};

		/**
		 * @brief Retrieve a string describing the engine.
		 * - `RENDERAPI` is either "Vulkan" or "OpenGL" depending on CMake configuration.
		 * - `BUILDCONFIG` is either "Debug" or "Release" depending on CMake configuration.
		 * @return 'Topaz vX.Y.Z `RENDERAPI` `BUILDCONFIG`' 
		 */
		std::string to_string() const;
		constexpr bool operator==(const EngineInfo& rhs) const = default;

		RenderAPI renderer;
		BuildConfig build;
		Version version;
	};

	/**
	 * @brief Retrieves the EngineInfo for this specific build of Topaz.
	 * @relates EngineInfo
	 * 
	 * @return Information about this Topaz build.
	 */
	constexpr EngineInfo info();

	/**
	 * @}
	 */
}
#include "tz/core/engine_info.inl"
#endif // TOPAZ_CORE_ENGINE_INFO_HPP
