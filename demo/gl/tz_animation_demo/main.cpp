#include "tz/tz.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/time.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/core/matrix_transform.hpp"

#include "tz/ren/animation.hpp"
struct dbgui_data_t
{
	bool mesh_renderer_enabled = false;
} dbgui_data;
void dbgui_init();

int main()
{
	tz::initialise
	({
		.name = "tz_animation_demo"
	});
	{
		dbgui_init();

		tz::ren::animation_renderer ar;
		ar.add_gltf(tz::io::gltf::from_file("../../demo/gl/tz_animation_demo/res/sponza.glb"));
		ar.append_to_render_graph();

		tz::duration update_timer = tz::system_time();
		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::begin_frame();
			// draw
			tz::gl::get_device().render();
			ar.update();
			update_timer = tz::system_time();
			// advance dbgui
			tz::dbgui::run([&ar]()
			{
				if(dbgui_data.mesh_renderer_enabled)
				{
					if(ImGui::Begin("Mesh Renderer", &dbgui_data.mesh_renderer_enabled))
					{
						ar.dbgui();
						ImGui::End();
					}
				}
			});
			tz::end_frame();
			TZ_FRAME_END;
		}
	}
	tz::terminate();
}

// invoked when dbgui needs to set up initial state.
void dbgui_init()
{
	tz::dbgui::game_menu().add_callback([]()
	{
		ImGui::MenuItem("Animation Renderer", nullptr, &dbgui_data.mesh_renderer_enabled);
	});
}