#ifndef TOPAZ_GL2_API_OUTPUT_HPP
#define TOPAZ_GL2_API_OUTPUT_HPP
#include "core/interfaces/cloneable.hpp"
#include <concepts>

namespace tz::gl
{
	enum class OutputTarget
	{
		Window,
		OffscreenImage
	};

	class IOutput : public IUniqueCloneable<IOutput>
	{
	public:
		constexpr virtual OutputTarget get_target() const = 0;
		virtual ~IOutput() = default;
	};
}

#endif // TOPAZ_GL2_API_OUTPUT_HPP
