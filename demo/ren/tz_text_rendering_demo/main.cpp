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

int main()
{
	tz::initialise(
		{
			.name = "tz_text_rendering_demo",
		});
	{
		dbgui_init();
		tz::io::ttf ttf = tz::io::ttf::from_memory(ImportedTextData(ProggyClean, ttf));
		tz::ren::text_renderer tren;
		tren.append_to_render_graph();
		tz::ren::text_renderer::font_handle fonth = tren.add_font(ttf);
		auto stringh = tren.add_string(fonth,
		tz::trs{
			.translate = {-500.0f, 0.0f, 0.0f},
			.rotate = tz::quat::from_axis_angle({0.0f, 0.0f, 1.0f}, 0.65f),
			.scale = tz::vec3::filled(25.0f)
		},"well met! i am harrand", {0.2f, 0.5f, 0.9f});

		tren.add_string(fonth,
		tz::trs{
			.translate = {-700.0f, -550.0f, 0.0f},
			.rotate = tz::quat::from_axis_angle({0.0f, 0.0f, 1.0f}, 0.65f),
			.scale = tz::vec3::filled(40.0f)
		},"you have (achieved) [({the})] rank of \":)\"", {0.2f, 0.5f, 0.9f});

		tren.add_string(fonth,
		tz::trs{
			.translate = {0.0f, -300.0f, 0.0f},
			.rotate = tz::quat::from_axis_angle({0.0f, 0.0f, 1.0f}, -0.26f),
			.scale = tz::vec3::filled(20.0f)
		},"congratulation. a winner is you", {0.9f, 0.5f, 0.3f});

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tren.update();
			tz::gl::get_device().render();

			static float counter = 0.01f;
			tren.string_set_colour(stringh, {1.0f, std::sin(counter += 0.01f), 1.0f - std::cos(counter)});
			tren.string_set_transform(stringh, tz::trs{{-800.0f, 0.0f, 0.0f}, {}, tz::vec3{50.0f, 50.0f, 50.0f} * 0.5f + std::sin(counter * 0.1f)});

			tz::dbgui::run([&ttf, &tren]()
			{
				if(dbgui_data.text_renderer_enabled)
				{
					if(ImGui::Begin("Text Renderer", &dbgui_data.text_renderer_enabled))
					{
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