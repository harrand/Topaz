#include "tz/tz.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/draw.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/dbgui/dbgui.hpp"

#include "tz/gl/imported_shaders.hpp"
#include ImportedShaderHeader(tz_gpu_driven_demo, compute)
#include ImportedShaderHeader(tz_gpu_driven_demo_render, vertex)
#include ImportedShaderHeader(tz_gpu_driven_demo_render, fragment)

int main()
{
	tz::initialise();
	{
		tz::gl::renderer_info cinfo;
		// we're doing draw-indirect-count, which means we must have a uint32 at the beginnign representing our count!
		constexpr std::size_t max_draw_count = 64;
		struct draw_indirect_count_data
		{
			std::uint32_t count;
			std::array<tz::gl::draw_indirect_command, max_draw_count> cmd;
		};
		tz::gl::resource_handle dbufh = cinfo.add_resource(tz::gl::buffer_resource::from_one(draw_indirect_count_data{},
		{
			.flags = {tz::gl::resource_flag::draw_indirect_buffer}
		}));
		tz::gl::resource_handle count_bufh = cinfo.add_resource(tz::gl::buffer_resource::from_one(std::uint32_t{1},
		{
			.access = tz::gl::resource_access::dynamic_fixed
		}));
		cinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(tz_gpu_driven_demo, compute));
		cinfo.debug_name("Compute Driver");
		tz::gl::renderer_handle ch = tz::gl::get_device().create_renderer(cinfo);

		tz::gl::renderer_info rinfo;
		rinfo.add_dependency(ch);
		rinfo.debug_name("Triangle renderer");
		tz::gl::resource_handle dbufh_ref = rinfo.ref_resource(ch, dbufh);
		rinfo.set_options({tz::gl::renderer_option::draw_indirect_count});
		rinfo.state().graphics.draw_buffer = dbufh_ref;
		rinfo.state().graphics.tri_count = 1;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_gpu_driven_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_gpu_driven_demo_render, fragment));
		tz::gl::renderer_handle rh = tz::gl::get_device().create_renderer(rinfo);

		tz::gl::get_device().render_graph().timeline = {ch, rh};

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device().render();
			tz::dbgui::run([ch, count_bufh]()
			{
				ImGui::Begin("#countdbgui");
				auto& count = tz::gl::get_device().get_renderer(ch).get_resource(count_bufh)->data_as<std::uint32_t>().front();
				ImGui::SliderInt("Draw Count", reinterpret_cast<int*>(&count), 1, max_draw_count, "%zu");
				ImGui::End();
			});
			tz::end_frame();
		}
	}
	tz::terminate();
}
