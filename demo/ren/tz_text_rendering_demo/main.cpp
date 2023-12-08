#include "tz/tz.hpp"
#include "tz/gl/device.hpp"
#include "tz/io/ttf.hpp"
#include "tz/ren/text.hpp"

#include "tz/core/imported_text.hpp"
#include ImportedTextHeader(ProggyClean, ttf)


// debug rendering
#include "tz/ren/mesh.hpp"

struct dbgui_data_t
{
	bool text_renderer_enabled = false;
} dbgui_data;
void dbgui_init();

static tz::io::ttf::rasterise_info rast
{
	.dimensions = {32u, 32u},
	.angle_threshold = 3.0f,
	.range = 0.1f,
	.scale = 64.0f,
	.translate = tz::vec2::zero()
};

constexpr char ch = 'F';

int main()
{
	tz::initialise({.name = "tz_text_rendering_demo"});
	{
		dbgui_init();
		tz::io::ttf ttf = tz::io::ttf::from_memory(ImportedTextData(ProggyClean, ttf));
		tz::io::image img_c = ttf.rasterise_msdf(ch, rast);
		tz::ren::text_renderer tren;
		tren.append_to_render_graph();
		tz::ren::text_renderer::font_handle fonth = tren.add_font(ttf);

		tz::ren::mesh_renderer mr;
		mr.append_to_render_graph();

		tz::ren::mesh_renderer::mesh quad;
		quad.vertices =
		{
			{.position = {0.5f, 0.5f, 0.0f}, .texcoord = {1.0f, 1.0f}},
			{.position = {0.5f, -0.5f, 0.0f}, .texcoord = {1.0f, 0.0f}},
			{.position = {-0.5f, -0.5f, 0.0f}, .texcoord = {0.0f, 0.0f}},
			{.position = {-0.5f, 0.5f, 0.0f}, .texcoord = {0.0f, 1.0f}}
		};	
		quad.indices = {0u, 1u, 3u, 1u, 2u, 3u};
		auto mesh = mr.add_mesh(quad);
		auto tex = mr.add_texture(img_c);
		mr.camera_perspective({.aspect_ratio = 1920.0f/1080.0f, .fov = 1.5708f});
		mr.set_camera_transform({.translate = {0.0f, 0.0f, 1.0f}});

		auto obj = mr.add_object
		({
			.mesh = mesh,
			.bound_textures = {{.texture = tex}}
		});

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device().render();
			tz::dbgui::run([&mr, &ttf, obj]()
			{
				if(dbgui_data.text_renderer_enabled)
				{
					if(ImGui::Begin("Text Renderer", &dbgui_data.text_renderer_enabled))
					{
						bool changed = false;
						changed |= ImGui::SliderInt2("Dimensions", reinterpret_cast<int*>(rast.dimensions.data().data()), 1u, 512u, "%u");
						changed |= ImGui::SliderFloat("Angle Threshold", &rast.angle_threshold, 0.0f, 5.0f);
						changed |= ImGui::SliderFloat("Range", &rast.range, 0.01f, 2.0f);
						changed |= ImGui::SliderFloat("Scale", &rast.scale, 0.01f, 64.0f);
						changed |= ImGui::SliderFloat2("Translate", rast.translate.data().data(), -1.0f, 1.0f);

						if(changed)
						{
							auto new_tex = mr.add_texture(ttf.rasterise_msdf(ch, rast));
							mr.get_object(obj).bound_textures[0].texture = new_tex;
						}

						ImGui::End();
					}
				}
			});
			tz::end_frame();
		}
	}
	tz::terminate();
}

// invoked when dbgui needs to set up initial state.
void dbgui_init()
{
	tz::dbgui::game_menu().add_callback([]()
	{
		ImGui::MenuItem("Text Renderer", nullptr, &dbgui_data.text_renderer_enabled);
	});
}