#include "tz/core/tz.hpp"
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
		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_triangle_demo, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_triangle_demo, fragment));
		rinfo.set_options({tz::gl::renderer_option::NoDepthTesting});

		tz::gl::renderer_handle renh = tz::gl::get_device().create_renderer(rinfo);
		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device().get_renderer(renh).render(1);
			tz::end_frame();
		}
	}
	tz::terminate();
}
