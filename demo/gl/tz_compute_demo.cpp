#include "tz/tz.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/gl/resource.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(tz_compute_demo, compute)
#include ImportedShaderHeader(tz_compute_demo_render, vertex)
#include ImportedShaderHeader(tz_compute_demo_render, fragment)

int main()
{
	tz::initialise
	({
		.name = "tz_compute_demo",
	});
	{
		tz::gl::buffer_resource colour_buffer = tz::gl::buffer_resource::from_many
		(
		 	{
				tz::vec4{0.0f, 0.0f, 0.0f, 1.0f},
				tz::vec4{1.0f, 0.0f, 0.0f, 1.0f},
				tz::vec4{0.0f, 1.0f, 0.0f, 1.0f},
				tz::vec4{0.0f, 0.0f, 1.0f, 1.0f}
			}
		);
		tz::gl::buffer_resource time_buffer = tz::gl::buffer_resource::from_one(0u);

		tz::gl::renderer_info pinfo;
		pinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(tz_compute_demo, compute));
		auto cbuf = pinfo.add_resource(colour_buffer);
		tz::gl::resource_handle tbufh = pinfo.add_resource(time_buffer);

		tz::gl::renderer_handle compute_workerh = tz::gl::get_device2().create_renderer(pinfo);

		tz::gl::renderer_info rinfo;
		rinfo.state().graphics.tri_count = 1;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_compute_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_compute_demo_render, fragment));
		tz::gl::resource_handle refbuf = rinfo.ref_resource(compute_workerh, cbuf);
		rinfo.debug_name("Window renderer");
		tz::gl::renderer_handle rendererh = tz::gl::get_device2().create_renderer(rinfo);

		tz::gl::renderer2& compute_worker = tz::gl::get_device2().get_renderer(compute_workerh);
		tz::gl::renderer2& renderer = tz::gl::get_device2().get_renderer(rendererh);

		tz::gl::get_device2().render_graph().timeline = {compute_workerh, rendererh};
		tz::gl::get_device2().render_graph().add_dependencies(rendererh, compute_workerh);

		bool game_menu_enabled = false;
		tz::dbgui::game_menu().add_callback([&game_menu_enabled]()
		{
			ImGui::MenuItem("Timing", nullptr, &game_menu_enabled);
		});

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device2().render();

			tz::dbgui::run([&game_menu_enabled, &compute_worker, &tbufh]()
			{
				if(game_menu_enabled)
				{
					ImGui::Begin("Compute Timing", &game_menu_enabled);
					ImGui::Text("Timer: %g", compute_worker.get_resource(tbufh)->data_as<float>().front());
					if(ImGui::Button("Reset Timer"))
					{
						tz::gl::RendererEditBuilder edit;
						std::array<float, 1> zdata{0.0f};
						std::span<float> zspan = zdata;
						edit.write
						({
							.resource = tbufh,
							.data = std::as_bytes(zspan)
						});
						compute_worker.edit(edit.build());
					}
					ImGui::End();
				}
			});

			tz::end_frame();
		}
	}
	tz::terminate();
}
