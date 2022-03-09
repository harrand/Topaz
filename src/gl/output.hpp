#ifndef TOPAZ_GL2_OUTPUT_HPP
#define TOPAZ_GL2_OUTPUT_HPP
#include "core/window.hpp"
#include "gl/api/output.hpp"
#include "gl/component.hpp"

namespace tz::gl
{
	class ImageOutput : public IOutput
	{
	public:
		ImageOutput(ImageComponent& component);
		constexpr virtual OutputTarget get_target() const override
		{
			return OutputTarget::OffscreenImage;
		}

		const ImageComponent& get_component() const;
		ImageComponent& get_component();
	private:
		// TODO: Span of components when we go on to support multiple-render-targets?
		ImageComponent* component;
	};

	class WindowOutput : public IOutput
	{
	public:
		WindowOutput(const tz::Window& window);
		constexpr virtual OutputTarget get_target() const override
		{
			return OutputTarget::Window;
		}

		const tz::Window& get_window() const;
	private:
		const tz::Window* wnd;
	};
}

#endif // TOPAZ_GL2_OUTPUT_HPP
