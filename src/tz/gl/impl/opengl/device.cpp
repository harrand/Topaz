#if TZ_OGL
#include "tz/core/profile.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"
#include "tz/gl/impl/opengl/device.hpp"

namespace tz::gl
{

	device_ogl::device_ogl()
	{
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	tz::gl::renderer_handle device_ogl::create_renderer(const renderer_info& info)
	{
		TZ_PROFZONE("OpenGL Frontend - renderer Create (via device)", 0xFFAA0000);
		return device_common<renderer_ogl>::emplace_renderer(info);
	}
	
	image_format device_ogl::get_window_format() const
	{
		return image_format::RGBA32;
	}

	void device_ogl::dbgui()
	{
		tz::gl::common_device_dbgui(*this);
	}

	void device_ogl::begin_frame()
	{

	}

	void device_ogl::end_frame()
	{

	}
}

#endif // TZ_OGL
