#if TZ_OGL
#include "tz/core/profiling/zone.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/frontend/ogl2/renderer.hpp"
#include "tz/gl/impl/frontend/ogl2/device.hpp"

namespace tz::gl
{

	DeviceOGL::DeviceOGL()
	{
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	tz::gl::RendererHandle DeviceOGL::create_renderer(const RendererInfoOGL& info)
	{
		TZ_PROFZONE("OpenGL Frontend - Renderer Create (via Device)", TZ_PROFCOL_RED);
		return DeviceCommon<RendererOGL>::emplace_renderer(info);
	}
	
	ImageFormat DeviceOGL::get_window_format() const
	{
		return ImageFormat::RGBA32;
	}
}

#endif // TZ_OGL
