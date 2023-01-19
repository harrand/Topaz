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
		tz::gl::resource_handle dbufh = cinfo.add_resource(tz::gl::BufferResource::from_one(tz::gl::DrawIndirectCommand{},
		{
			.flags = {tz::gl::resource_flag::draw_indirect_buffer}
		}));
		cinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(tz_gpu_driven_demo, compute));
		cinfo.debug_name("Compute Driver");
		tz::gl::renderer_handle ch = tz::gl::device().create_renderer(cinfo);

		tz::gl::renderer_info rinfo;
		rinfo.debug_name("Triangle Renderer");
		tz::gl::resource_handle dbufh_ref = rinfo.ref_resource(ch, dbufh);
		rinfo.state().graphics.draw_buffer = dbufh_ref;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_gpu_driven_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_gpu_driven_demo_render, fragment));
		tz::gl::renderer_handle rh = tz::gl::device().create_renderer(rinfo);

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::device().get_renderer(ch).render();
			tz::gl::device().get_renderer(rh).render(1);	
			tz::end_frame();
		}
	}
	tz::terminate();
}
