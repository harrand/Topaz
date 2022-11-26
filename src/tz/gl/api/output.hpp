#ifndef TOPAZ_GL2_API_OUTPUT_HPP
#define TOPAZ_GL2_API_OUTPUT_HPP
#include "hdk/memory/clone.hpp"
#include "hdk/data/vector.hpp"
#include <concepts>
#include <limits>

namespace tz::gl
{
	enum class OutputTarget
	{
		Window,
		OffscreenImage
	};

	struct ViewportRegion
	{
		hdk::vec2ui offset;
		hdk::vec2ui extent;

		static ViewportRegion null()
		{
			return {.offset = {0u, 0u}, .extent = {std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()}};
		}

		bool operator==(const ViewportRegion& rhs) const = default;
	};

	struct ScissorRegion
	{
		hdk::vec2ui offset;
		hdk::vec2ui extent;

		static ScissorRegion null()
		{
			return {.offset = {0u, 0u}, .extent = {std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()}};
		}

		bool operator==(const ScissorRegion& rhs) const = default;
	};

	class IOutput : public hdk::unique_cloneable<IOutput>
	{
	public:
		constexpr virtual OutputTarget get_target() const = 0;
		virtual ~IOutput() = default;

		ViewportRegion viewport = ViewportRegion::null();
		ScissorRegion scissor = ScissorRegion::null();
	};
}

#endif // TOPAZ_GL2_API_OUTPUT_HPP
