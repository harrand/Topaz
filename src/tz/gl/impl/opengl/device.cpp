#if TZ_OGL
#include "hdk/profile.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/gl/impl/opengl/renderer.hpp"
#include "tz/gl/impl/opengl/device.hpp"

namespace tz::gl
{

	device_ogl::device_ogl()
	{
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	}

	tz::gl::renderer_handle device_ogl::create_renderer(const renderer_info_ogl& info)
	{
		HDK_PROFZONE("OpenGL Frontend - renderer Create (via device)", 0xFFAA0000);
		return DeviceCommon<renderer_ogl>::emplace_renderer(info);
	}
	
	image_format device_ogl::get_window_format() const
	{
		return image_format::RGBA32;
	}

	void device_ogl::dbgui()
	{
		tz::gl::common_device_dbgui(*this);
	}
}

#endif // TZ_OGL
