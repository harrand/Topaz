#include "tz/core/tz.hpp"
#include "tz/gl/renderer.hpp"
#include "tz/gl/device.hpp"
#include "tz/gl/resource.hpp"
#include "tz/gl/imported_shaders.hpp"
#include "tz/dbgui/dbgui.hpp"

#include ImportedShaderHeader(tz_bloom_demo, vertex)
#include ImportedShaderHeader(tz_bloom_demo, fragment)

struct RenderData
{
	tz::Vec2 triangle_pos = {-0.5f, 0.0f};
	tz::Vec2 triangle_scale = {1.0f, 1.0f};
	tz::Vec2 quad_pos = {0.5f, 0.0f};
	tz::Vec2 quad_scale = {1.0f, 1.0f};

	tz::Vec3 triangle_colour = {1.0f, 0.0f, 0.0f};
	float pad0;
	tz::Vec3 quad_colour = {0.0f, 0.0f, 1.0f};
	float pad1;
};

void dbgui(RenderData& data)
{
	if(ImGui::CollapsingHeader("Triangle"))
	{
		ImGui::DragFloat2("Position", data.triangle_pos.data().data(), 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat2("Scale", data.triangle_scale.data().data(), 0.005f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Colour", data.triangle_colour.data().data());
	}
	if(ImGui::CollapsingHeader("Square"))
	{
		ImGui::DragFloat2("Position", data.quad_pos.data().data(), 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat2("Scale", data.quad_scale.data().data(), 0.005f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Colour", data.quad_colour.data().data());
	}
}

int main()
{

	tz::initialise
	({
		.name = "tz_bloom_demo",
	});
	{
		// Firstly draw some shapes. Brighter pixels are written into a second colour attachment
		tz::gl::BufferResource render_data = tz::gl::BufferResource::from_one(RenderData{}, tz::gl::ResourceAccess::DynamicFixed);

		tz::gl::RendererInfo rinfo;
		rinfo.shader().set_shader(tz::gl::ShaderStage::Vertex, ImportedShaderSource(tz_bloom_demo, vertex));
		rinfo.shader().set_shader(tz::gl::ShaderStage::Fragment, ImportedShaderSource(tz_bloom_demo, fragment));
		tz::gl::ResourceHandle render_bufh = rinfo.add_resource(render_data);
		tz::gl::Renderer renderer = tz::gl::device().create_renderer(rinfo);

		// Blur the second colour attachment
		// TODO
		
		// Do a final pass, essentially adding both images together. Present that image
		// TODO

		// Debug UI
		bool menu_enabled = false;
		tz::dbgui::game_menu().add_callback([&menu_enabled]()
		{
			ImGui::MenuItem("Shapes", nullptr, &menu_enabled);
		});

		while(!tz::window().is_close_requested())
		{
			tz::window().begin_frame();
			renderer.render(3);

			tz::dbgui::run([&menu_enabled, &render_bufh, &renderer]()
			{
				if(menu_enabled)
				{
					ImGui::Begin("Shapes", &menu_enabled);
					dbgui(renderer.get_resource(render_bufh)->data_as<RenderData>().front());
					ImGui::End();
				}
			});
			tz::window().end_frame();
		}
	}
	tz::terminate();
}
