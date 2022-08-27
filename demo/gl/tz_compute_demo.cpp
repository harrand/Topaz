#include "tz/core/tz.hpp"
#include "tz/core/window.hpp"
#include "tz/core/profiling/zone.hpp"
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
		({
			tz::Vec4{0.0f, 0.0f, 0.0f, 1.0f},
			tz::Vec4{1.0f, 0.0f, 0.0f, 1.0f},
			tz::Vec4{0.0f, 1.0f, 0.0f, 1.0f},
			tz::Vec4{0.0f, 0.0f, 1.0f, 1.0f}
		});
		tz::gl::BufferResource time_buffer = tz::gl::BufferResource::from_one(0u);

		tz::gl::RendererInfo pinfo;
		pinfo.shader().set_shader(tz::gl::ShaderStage::Compute, ImportedShaderSource(tz_compute_demo, compute));
		auto cbuf = pinfo.add_resource(colour_buffer);
		tz::gl::ResourceHandle tbufh = pinfo.add_resource(time_buffer);
		pinfo.debug_name("Compute");

		tz::gl::Renderer compute_worker = tz::gl::device().create_renderer(pinfo);

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_compute_demo_render, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_compute_demo_render, fragment));
		auto refbuf = rinfo.add_component(*compute_worker.get_component(cbuf));
		rinfo.debug_name("Window Renderer");
		tz::gl::Renderer renderer = tz::gl::device().create_renderer(rinfo);

		bool game_menu_enabled = false;
		tz::dbgui::game_menu().add_callback([&game_menu_enabled]()
		{
			ImGui::MenuItem("Timing", nullptr, &game_menu_enabled);
		});

		while(!tz::window().is_close_requested())
		{
			tz::window().begin_frame();
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

			tz::window().end_frame();
		}
	}
	tz::terminate();
}
