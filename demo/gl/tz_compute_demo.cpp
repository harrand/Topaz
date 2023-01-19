#include "tz/core/tz.hpp"
#include "hdk/profile.hpp"
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
		tz::gl::BufferResource colour_buffer = tz::gl::BufferResource::from_many
		(
		 	{
				hdk::vec4{0.0f, 0.0f, 0.0f, 1.0f},
				hdk::vec4{1.0f, 0.0f, 0.0f, 1.0f},
				hdk::vec4{0.0f, 1.0f, 0.0f, 1.0f},
				hdk::vec4{0.0f, 0.0f, 1.0f, 1.0f}
			}
		);
		tz::gl::BufferResource time_buffer = tz::gl::BufferResource::from_one(0u);

		tz::gl::renderer_info pinfo;
		pinfo.shader().set_shader(tz::gl::shader_stage::compute, ImportedShaderSource(tz_compute_demo, compute));
		auto cbuf = pinfo.add_resource(colour_buffer);
		tz::gl::resource_handle tbufh = pinfo.add_resource(time_buffer);

		tz::gl::renderer_handle compute_workerh = tz::gl::get_device().create_renderer(pinfo);

		tz::gl::renderer_info rinfo;
		rinfo.shader().set_shader(tz::gl::shader_stage::vertex, ImportedShaderSource(tz_compute_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::shader_stage::fragment, ImportedShaderSource(tz_compute_demo_render, fragment));
		tz::gl::resource_handle refbuf = rinfo.ref_resource(compute_workerh, cbuf);
		rinfo.debug_name("Window renderer");
		tz::gl::renderer_handle rendererh = tz::gl::get_device().create_renderer(rinfo);

		tz::gl::renderer& compute_worker = tz::gl::get_device().get_renderer(compute_workerh);
		tz::gl::renderer& renderer = tz::gl::get_device().get_renderer(rendererh);

		bool game_menu_enabled = false;
		tz::dbgui::game_menu().add_callback([&game_menu_enabled]()
		{
			ImGui::MenuItem("Timing", nullptr, &game_menu_enabled);
		});

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			compute_worker.render();
			renderer.render(1);

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
