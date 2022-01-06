#ifndef TOPAZ_GL2_API_OUTPUT_HPP
#define TOPAZ_GL2_API_OUTPUT_HPP
#include <concepts>

namespace tz::gl2
{
	enum class OutputTarget
	{
		Window,
		OffscreenImage
	};

	class IOutput
	{
	public:
		constexpr virtual OutputTarget get_target() const = 0;
	};
}

#endif // TOPAZ_GL2_API_OUTPUT_HPP
