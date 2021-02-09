#ifndef TOPAZ_GL_IMGUI_OGL_INFO_HPP
#define TOPAZ_GL_IMGUI_OGL_INFO_HPP
#include "dui/window.hpp"

namespace tz::dui::gl
{
	class OpenGLInfoWindow : public tz::dui::DebugWindow
	{
	public:
		OpenGLInfoWindow();
		virtual void render() override;
	};
}

#endif // TOPAZ_GL_IMGUI_OGL_INFO_HPP