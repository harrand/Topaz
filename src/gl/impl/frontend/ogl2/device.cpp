#include "gl/impl/frontend/ogl2/renderer.hpp"
#if TZ_OGL
#include "gl/impl/frontend/ogl2/device.hpp"

namespace tz::gl
{

	RendererOGL DeviceOGL::create_renderer(RendererInfoOGL info)
	{
		return {info};
	}
	
	ImageFormat DeviceOGL::get_window_format() const
	{
		return ImageFormat::RGBA32;
	}
}

#endif // TZ_OGL
