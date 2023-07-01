#include "tz/tz.hpp"
#include "tz/core/imported_text.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/io/gltf.hpp"
#include "mesh_renderer.hpp"

#include ImportedTextHeader(cube, glb)

struct dbgui_data_t
{
	bool mesh_renderer_enabled = false;
} dbgui_data;
void dbgui_init();
mesh_t create_cube_mesh(float sz);
mesh_t temp_debug_load_cube_gltf();

int main()
{
	tz::initialise
	({
		.name = "tz_mesh_demo"
	});
	{
		dbgui_init();
		temp_debug_load_cube_gltf();

		mesh_renderer renderer{8u};
		renderer.push_back_timeline();
		meshid_t cube1 = renderer.add_mesh(temp_debug_load_cube_gltf(), "Small Cube");
		meshid_t cube2 = renderer.add_mesh(create_cube_mesh(0.5f), "Big Cube");
		meshid_t cube3 = renderer.add_mesh(create_cube_mesh(2.5f), "Huge Cube");
		std::vector<std::byte> black_texdata
		{
			std::byte{0},
			std::byte{0},
			std::byte{0},
			std::byte{0},
		};
		// pure white.
		std::vector<std::byte> white_texdata
		{
			std::byte{255},
			std::byte{255},
			std::byte{255},
			std::byte{255},
		};

		// missingtex.
		std::vector<std::byte> texdata
		{
			std::byte{255},
			std::byte{255},
			std::byte{255},
			std::byte{255},

			std::byte{128},
			std::byte{128},
			std::byte{128},
			std::byte{255},

			std::byte{128},
			std::byte{128},
			std::byte{128},
			std::byte{255},

			std::byte{255},
			std::byte{255},
			std::byte{255},
			std::byte{255},
		};
		texid_t blk = renderer.add_texture(1, 1, black_texdata);
		texid_t whte = renderer.add_texture(1, 1, white_texdata);
		texid_t missingtex = renderer.add_texture(2, 2, texdata);

		renderer.add_to_draw_list(cube2, {.pos = {1.0f, 0.0f, 0.0f}}, whte);
		renderer.add_to_draw_list(cube1, {.pos = {-2.0f, 0.0f, 0.0f}}, missingtex);

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
			// BENNYBOX
			vertex_t{.pos = {-sz, -sz, -sz}, .texc = {1.0f, 0.0f}, .nrm = {0.0f, 0.0f, -1.0f}},
			vertex_t{.pos = {-sz, sz, -sz}, .texc = {0.0f, 0.0f}, .nrm = {0.0f, 0.0f, -1.0f}},
			vertex_t{.pos = {sz, sz, -sz}, .texc = {0.0f, 1.0f}, .nrm = {0.0f, 0.0f, -1.0f}},
			vertex_t{.pos = {sz, -sz, -sz}, .texc = {1.0f, 1.0f}, .nrm = {0.0f, 0.0f, -1.0f}},

			vertex_t{.pos = {-sz, -sz, sz}, .texc = {1.0f, 0.0f}, .nrm = {0.0f, 0.0f, 1.0f}},
			vertex_t{.pos = {-sz, sz, sz}, .texc = {0.0f, 0.0f}, .nrm = {0.0f, 0.0f, 1.0f}},
			vertex_t{.pos = {sz, sz, sz}, .texc = {0.0f, 1.0f}, .nrm = {0.0f, 0.0f, 1.0f}},
			vertex_t{.pos = {sz, -sz, sz}, .texc = {1.0f, 1.0f}, .nrm = {0.0f, 0.0f, 1.0f}},

			vertex_t{.pos = {-sz, -sz, -sz}, .texc = {0.0f, 1.0f}, .nrm = {0.0f, -1.0f, 0.0f}},
			vertex_t{.pos = {-sz, -sz, sz}, .texc = {1.0f, 1.0f}, .nrm = {0.0f, -1.0f, 0.0f}},
			vertex_t{.pos = {sz, -sz, sz}, .texc = {1.0f, 0.0f}, .nrm = {0.0f, -1.0f, 0.0f}},
			vertex_t{.pos = {sz, -sz, -sz}, .texc = {0.0f, 0.0f}, .nrm = {0.0f, -1.0f, 0.0f}},

			vertex_t{.pos = {-sz, sz, -sz}, .texc = {0.0f, 1.0f}, .nrm = {0.0f, 1.0f, 0.0f}},
			vertex_t{.pos = {-sz, sz, sz}, .texc = {1.0f, 1.0f}, .nrm = {0.0f, 1.0f, 0.0f}},
			vertex_t{.pos = {sz, sz, sz}, .texc = {1.0f, 0.0f}, .nrm = {0.0f, 1.0f, 0.0f}},
			vertex_t{.pos = {sz, sz, -sz}, .texc = {0.0f, 0.0f}, .nrm = {0.0f, 1.0f, 0.0f}},

			vertex_t{.pos = {-sz, -sz, -sz}, .texc = {1.0f, 1.0f}, .nrm = {-1.0f, 0.0f, 0.0f}},
			vertex_t{.pos = {-sz, -sz, sz}, .texc = {1.0f, 0.0f}, .nrm = {-1.0f, 0.0f, 0.0f}},
			vertex_t{.pos = {-sz, sz, sz}, .texc = {0.0f, 0.0f}, .nrm = {-1.0f, 0.0f, 0.0f}},
			vertex_t{.pos = {-sz, sz, -sz}, .texc = {0.0f, 1.0f}, .nrm = {-1.0f, 0.0f, 0.0f}},

			vertex_t{.pos = {sz, -sz, -sz}, .texc = {1.0f, 1.0f}, .nrm = {1.0f, 0.0f, 0.0f}},
			vertex_t{.pos = {sz, -sz, sz}, .texc = {1.0f, 0.0f}, .nrm = {1.0f, 0.0f, 0.0f}},
			vertex_t{.pos = {sz, sz, sz}, .texc = {0.0f, 0.0f}, .nrm = {1.0f, 0.0f, 0.0f}},
			vertex_t{.pos = {sz, sz, -sz}, .texc = {0.0f, 1.0f}, .nrm = {1.0f, 0.0f, 0.0f}},
		},
		.indices =
		{
			0, 1, 2,
			0, 2, 3,

			6, 5, 4,
			7, 6, 4,

			10, 9, 8,
			11, 10, 8,

			12, 13, 14,
			12, 14, 15,

			16, 17, 18,
			16, 18, 19,

			22, 21, 20,
			23, 22, 20
		}
	};
}

mesh_t temp_debug_load_cube_gltf()
{
	tz::io::gltf cube = tz::io::gltf::from_memory(ImportedTextData(cube, glb));
	tz::io::gltf_submesh_data cube_meshdata = cube.get_submesh_vertex_data(0, 0);

	mesh_t ret;
	ret.vertices.resize(cube_meshdata.vertices.size());
	std::transform(cube_meshdata.vertices.begin(), cube_meshdata.vertices.end(), ret.vertices.begin(),
	[](tz::io::gltf_vertex_data gltf_vtx) -> vertex_t
	{
		return
		{
			.pos = gltf_vtx.position,
			.texc = gltf_vtx.texcoordn[0],
			.nrm = gltf_vtx.normal,
			.tang = gltf_vtx.tangent,
		};
	});
	ret.indices = cube_meshdata.indices;
	return ret;
}
