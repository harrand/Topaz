#include "gl/output.hpp"

namespace tz::gl2
{
	ImageOutput::ImageOutput(ImageComponent& component):
	component(&component){}

	const ImageComponent& ImageOutput::get_component() const
	{
		return *this->component;
	}

	ImageComponent& ImageOutput::get_component()
	{
		return *this->component;
	}

	WindowOutput::WindowOutput(const tz::Window& window):
	wnd(&window){}

	const tz::Window& WindowOutput::get_window() const
	{
		return *this->wnd;
	}
}
