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
	tz::initialise({.name = "tz_text_rendering_demo"});
	{
		dbgui_init();
		tz::io::ttf ttf = tz::io::ttf::from_memory(ImportedTextData(ProggyClean, ttf));
		tz::ren::text_renderer tren;
		tren.append_to_render_graph();
		tz::ren::text_renderer::font_handle fonth = tren.add_font(ttf);
		tren.add_string(fonth, tz::vec2{1.0f, 2.0f},"well met! i am harrand");
		tren.add_string(fonth, tz::vec2{3.0f, 4.0f},"big jon was here.");

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			tz::gl::get_device().render();
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