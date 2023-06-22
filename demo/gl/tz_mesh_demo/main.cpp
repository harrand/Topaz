#include "tz/tz.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "mesh_renderer.hpp"

struct dbgui_data_t
{
	bool mesh_renderer_enabled = false;
} dbgui_data;
void dbgui_init();
mesh_t create_cube_mesh(float sz);

int main()
{
	tz::initialise
	({
		.name = "tz_mesh_demo"
	});
	{
		dbgui_init();

		mesh_renderer renderer;
		renderer.push_back_timeline();
		renderer.add_mesh(create_cube_mesh(0.1f), "Small Cube");
		renderer.add_mesh(create_cube_mesh(0.5f), "Big Cube");
		renderer.add_mesh(create_cube_mesh(2.5f), "Huge Cube");

		while(!tz::window().is_close_requested())
		{
			tz::begin_frame();
			// draw
			tz::gl::get_device().render();
			// advance dbgui
			tz::dbgui::run([&renderer]()
			{
				if(dbgui_data.mesh_renderer_enabled)
				{
					ImGui::Begin("Mesh Renderer", &dbgui_data.mesh_renderer_enabled);
					renderer.dbgui();
					ImGui::End();
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
		ImGui::MenuItem("Mesh Renderer", nullptr, &dbgui_data.mesh_renderer_enabled);
	});
}

mesh_t create_cube_mesh(float sz)
{
	return mesh_t
	{
		.vertices =
		{
			// Front face
			vertex_t{ tz::vec3{-sz, -sz, sz}, tz::vec2{0.0f, 0.0f}, tz::vec3{0.0f, 0.0f, 1.0f}, tz::vec3{} },  // Bottom-left
			vertex_t{ tz::vec3{sz, -sz, sz}, tz::vec2{1.0f, 0.0f}, tz::vec3{0.0f, 0.0f, 1.0f}, tz::vec3{} },   // Bottom-right
			vertex_t{ tz::vec3{sz, sz, sz}, tz::vec2{1.0f, 1.0f}, tz::vec3{0.0f, 0.0f, 1.0f}, tz::vec3{} },    // Top-right
			vertex_t{ tz::vec3{-sz, sz, sz}, tz::vec2{0.0f, 1.0f}, tz::vec3{0.0f, 0.0f, 1.0f}, tz::vec3{} },   // Top-left

			// Back face
			vertex_t{ tz::vec3{-sz, -sz, -sz}, tz::vec2{1.0f, 0.0f}, tz::vec3{0.0f, 0.0f, -1.0f}, tz::vec3{} }, // Bottom-left
			vertex_t{ tz::vec3{-sz, sz, -sz}, tz::vec2{1.0f, 1.0f}, tz::vec3{0.0f, 0.0f, -1.0f}, tz::vec3{} },  // Top-left
			vertex_t{ tz::vec3{sz, sz, -sz}, tz::vec2{0.0f, 1.0f}, tz::vec3{0.0f, 0.0f, -1.0f}, tz::vec3{} },   // Top-right
			vertex_t{ tz::vec3{sz, -sz, -sz}, tz::vec2{0.0f, 0.0f}, tz::vec3{0.0f, 0.0f, -1.0f}, tz::vec3{} }   // Bottom-right
		},
		.indices =
		{
			0, 1, 2, 2, 3, 0,       // Front face
			1, 4, 7, 7, 2, 1,       // Right face
			4, 5, 6, 6, 7, 4,       // Back face
			5, 0, 3, 3, 6, 5,       // Left face
			3, 2, 7, 7, 6, 3,       // Top face
			1, 0, 5, 5, 4, 1        // Bottom face
		}
	};
}
