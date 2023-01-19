#ifndef TOPAZ_GL2_API_OUTPUT_HPP
#define TOPAZ_GL2_API_OUTPUT_HPP
#include "tz/core/memory/clone.hpp"
#include "tz/core/data/vector.hpp"
#include <concepts>
#include <limits>

namespace tz::gl
{
	enum class output_target
	{
		window,
		offscreen_image
	};

	struct viewport_region
	{
		tz::vec2ui offset;
		tz::vec2ui extent;

		static viewport_region null()
		{
			return {.offset = {0u, 0u}, .extent = {std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()}};
		}

		bool operator==(const viewport_region& rhs) const = default;
	};

	struct scissor_region
	{
		tz::vec2ui offset;
		tz::vec2ui extent;

		static scissor_region null()
		{
			return {.offset = {0u, 0u}, .extent = {std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()}};
		}

		bool operator==(const scissor_region& rhs) const = default;
	};

	class ioutput : public tz::unique_cloneable<ioutput>
	{
	public:
		constexpr virtual output_target get_target() const = 0;
		virtual ~ioutput() = default;

		viewport_region viewport = viewport_region::null();
		scissor_region scissor = scissor_region::null();
	};
}

#endif // TOPAZ_GL2_API_OUTPUT_HPP
