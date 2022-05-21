#if TZ_OGL
#include "core/profiling/zone.hpp"
#include "gl/impl/frontend/ogl2/renderer.hpp"
#include "gl/impl/frontend/ogl2/device.hpp"

namespace tz::gl
{

	RendererOGL DeviceOGL::create_renderer(const RendererInfoOGL& info)
	{
		TZ_PROFZONE("OpenGL Frontend - Renderer Create (via Device)", TZ_PROFCOL_RED);
		return {info};
	}
	
	ImageFormat DeviceOGL::get_window_format() const
	{
		return ImageFormat::RGBA32;
	}
}

#endif // TZ_OGL
