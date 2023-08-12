#include "tz/tz.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/time.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/io/gltf.hpp"

#include "tz/ren/mesh.hpp"
struct dbgui_data_t
{
	bool mesh_renderer_enabled = false;
} dbgui_data;
void dbgui_init();

int main()
{
	tz::initialise
	({
		.name = "tz_mesh_demo"
	});
	{
		dbgui_init();

		tz::ren::mesh_renderer mr;
		tz::ren::mesh_renderer::mesh_t m;
		m.indices = {0u, 1u, 2u};
		m.vertices = 
		{
			{.position = {-0.5f, -0.5f, 0.0f}},
			{.position = {0.5f, -0.5f, 0.0f}},
			{.position = {-0.5f, 0.5f, 0.0f}}
		};
		mr.add_mesh(m);
		mr.append_to_render_graph();

		tz::duration update_timer = tz::system_time();
		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::begin_frame();
			// draw
			tz::gl::get_device().render();
			update_timer = tz::system_time();
			// advance dbgui
			tz::dbgui::run([&mr]()
			{
				if(dbgui_data.mesh_renderer_enabled)
				{
					if(ImGui::Begin("Mesh Renderer", &dbgui_data.mesh_renderer_enabled))
					{
						mr.dbgui();
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
		ImGui::MenuItem("Mesh Renderer", nullptr, &dbgui_data.mesh_renderer_enabled);
	});
}