#ifndef TOPAZ_GL2_API_OUTPUT_HPP
#define TOPAZ_GL2_API_OUTPUT_HPP
#include "hdk/memory/clone.hpp"
#include "hdk/data/vector.hpp"
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
		hdk::vec2ui offset;
		hdk::vec2ui extent;

		static viewport_region null()
		{
			return {.offset = {0u, 0u}, .extent = {std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()}};
		}

		bool operator==(const viewport_region& rhs) const = default;
	};

	struct scissor_region
	{
		hdk::vec2ui offset;
		hdk::vec2ui extent;

		static scissor_region null()
		{
			return {.offset = {0u, 0u}, .extent = {std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()}};
		}

		bool operator==(const scissor_region& rhs) const = default;
	};

	class ioutput : public hdk::unique_cloneable<ioutput>
	{
	public:
		constexpr virtual output_target get_target() const = 0;
		virtual ~ioutput() = default;

		viewport_region viewport = viewport_region::null();
		scissor_region scissor = scissor_region::null();
	};
}

#endif // TOPAZ_GL2_API_OUTPUT_HPP
