#if TZ_OGL
#include "hdk/profile.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"
#include "tz/gl/impl/opengl/device.hpp"

namespace tz::gl
{

	DeviceOGL::DeviceOGL()
	{
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	tz::gl::renderer_handle DeviceOGL::create_renderer(const renderer_infoOGL& info)
	{
		HDK_PROFZONE("OpenGL Frontend - Renderer Create (via device)", 0xFFAA0000);
		return DeviceCommon<RendererOGL>::emplace_renderer(info);
	}
	
	image_format DeviceOGL::get_window_format() const
	{
		return image_format::RGBA32;
	}

	void DeviceOGL::dbgui()
	{
		tz::gl::common_device_dbgui(*this);
	}
}

#endif // TZ_OGL
