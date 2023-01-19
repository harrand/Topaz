#include "tz/core/tz.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"

#include "tz/gl/imported_shaders.hpp"
#include ImportedShaderHeader(tz_gpu_driven_demo, compute)
#include ImportedShaderHeader(tz_gpu_driven_demo_render, vertex)
#include ImportedShaderHeader(tz_gpu_driven_demo_render, fragment)

int main()
{
	tz::initialise();
	{
		tz::gl::renderer_info cinfo;
		tz::gl::resource_handle dbufh = cinfo.add_resource(tz::gl::buffer_resource::from_one(tz::gl::draw_indirect_command{},
		{
			.flags = {tz::gl::resource_flag::draw_indirect_buffer}
		}));
		cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(tz_gpu_driven_demo, compute));
		cinfo.debug_name("Compute Driver");
		tz::gl::renderer_handle ch = tz::gl::get_device().create_renderer(cinfo);

		tz::gl::renderer_info rinfo;
		rinfo.debug_name("Triangle renderer");
		tz::gl::resource_handle dbufh_ref = rinfo.ref_resource(ch, dbufh);
		rinfo.state().graphics.draw_buffer = dbufh_ref;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_gpu_driven_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_gpu_driven_demo_render, fragment));
		tz::gl::renderer_handle rh = tz::gl::get_device().create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device().get_renderer(ch).render();
			tz::gl::get_device().get_renderer(rh).render(1);	
			tz::end_frame();
		}
	}
	tz::terminate();
}
