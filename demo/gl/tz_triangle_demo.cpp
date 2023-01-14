#include "tz/core/tz.hpp"
#include "tz/core/window.hpp"
#include "tz/wsi/wsi.hpp"
#include "hdk/profile.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(tz_triangle_demo, vertex)
#include ImportedShaderHeader(tz_triangle_demo, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_triangle_demo",
	});
	{
		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_triangle_demo, fragment));
		rinfo.set_options({tz::gl::RendererOption::NoDepthTesting});
		rinfo.state().graphics.clear_colour = hdk::vec4::filled(0.0f);

		tz::gl::RendererHandle renh = tz::gl::device().create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			HDK_FRAME_BEGIN;
			tz::dbgui::begin_frame();
			tz::gl::device().get_renderer(renh).render(1);
			tz::dbgui::end_frame();
			tz::wsi::update();
			HDK_FRAME_END;
		}
	}
	tz::terminate();
}
