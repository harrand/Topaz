#include "tz/tz.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/time.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/io/gltf.hpp"
#include "mesh_renderer.hpp"

#include "tz/ren/mesh.hpp"

struct dbgui_data_t
{
	bool mesh_renderer_enabled = false;
} dbgui_data;
void dbgui_init();

struct scene_t
{
	std::vector<mesh_t> meshes;
	std::vector<tz::io::image> images;
};

scene_t load_gltf(const char* path);

int main()
{
	tz::initialise
	({
		.name = "tz_mesh_demo"
	});
	{
		dbgui_init();

		tz::ren::mesh_renderer mr;
		mr.add_mesh({});

		scene_t scene = load_gltf("../../demo/gl/tz_mesh_demo/res/sponza.glb");
		std::vector<meshid_t> scene_meshes(scene.meshes.size());
		std::vector<texid_t> scene_images(scene.images.size());
		std::vector<std::vector<std::size_t>> mesh_bound_images(scene.meshes.size());

		mesh_renderer renderer{scene.images.size()};
		renderer.push_back_timeline();

		for(std::size_t i = 0; i < scene.meshes.size(); i++)
		{
			std::string mname = "GLTF Mesh" + std::to_string(i);
			scene_meshes[i] = renderer.add_mesh(scene.meshes[i], mname.c_str());
			std::size_t iid = 0;
			std::size_t nid = 0;
			if(scene.meshes[i].image_id != static_cast<std::size_t>(-1))
			{
				iid = scene.meshes[i].image_id;
			}
			if(scene.meshes[i].norm_image_id != static_cast<std::size_t>(-1))
			{
				nid = scene.meshes[i].norm_image_id;
			}
			mesh_bound_images[i] = {iid, nid};
		}
		for(std::size_t i = 0; i < scene.images.size(); i++)
		{
			//std::string iname = "GLTF Image" + std::to_string(i);
			const tz::io::image& img = scene.images[i];
			scene_images[i] = renderer.add_texture(img.width, img.height, img.data);
		}
		for(std::size_t i = 0; i < scene.meshes.size(); i++)
		{
			std::vector<texid_t> real_image_ids;
			for(std::size_t id : mesh_bound_images[i])
			{
				real_image_ids.push_back(scene_images[id]);
			}
			renderer.add_to_draw_list(scene_meshes[i], {.scale = {0.01f, 0.01f, 0.01f}}, real_image_ids);
		}

		tz::duration update_timer = tz::system_time();
		while(!tz::window().is_close_requested())
		{
			TZ_FRAME_BEGIN;
			tz::begin_frame();
			// draw
			tz::gl::get_device().render();
			renderer.update((tz::system_time() - update_timer).seconds<float>());
			update_timer = tz::system_time();
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

scene_t load_gltf(const char* path)
{
	TZ_PROFZONE("Load GLTF", 0xFF44DD44);
	scene_t ret;

	tz::io::gltf cube = tz::io::gltf::from_file(path);
	for(std::size_t i = 0; i < cube.get_meshes().size(); i++)
	{
		const tz::io::gltf_mesh& m = cube.get_meshes()[i];
		for(std::size_t j = 0; j < m.submeshes.size(); j++)
		{
			tz::io::gltf_submesh_data data = cube.get_submesh_vertex_data(i, j);
			mesh_t& cur = ret.meshes.emplace_back();
			cur.vertices.resize(data.vertices.size());
			cur.indices = data.indices;
			std::transform(data.vertices.begin(), data.vertices.end(), cur.vertices.begin(),
			[](const tz::io::gltf_vertex_data& gltf_vtx)->vertex_t
			{
				return
				{
					.pos = gltf_vtx.position,
					.texc = gltf_vtx.texcoordn[0],
					.nrm = gltf_vtx.normal,
					.tang = gltf_vtx.tangent,
				};
			});
			for(tz::io::gltf_submesh_texture_data tex : data.textures)
			{
				if(tex.type == tz::io::gltf_submesh_texture_type::color)
				{
					cur.image_id = tex.image_id;
					tz::assert(tex.texcoord_id == 0, "use of multiple texcoords is NYI. GLTF wants to use texcoord %zu for colour", tex.texcoord_id);
				}
				else if(tex.type == tz::io::gltf_submesh_texture_type::normal)
				{
					cur.norm_image_id = tex.image_id;
					tz::assert(tex.texcoord_id == 0, "use of multiple texcoords is NYI. GLTF wants to use texcoord %zu for colour", tex.texcoord_id);
				}
			}
		}
	}

	std::vector<tz::io::image> images;
	images.resize(cube.get_images().size());
	if(cube.get_images().size() > 8)
	{
		TZ_PROFZONE("Load Images - Jobs", 0xFF44DD44);
		// we should split this into threads.
		std::size_t job_count = std::thread::hardware_concurrency();
		std::size_t imgs_per_job = cube.get_images().size() / job_count; 
		std::size_t remainder_imgs = cube.get_images().size() % job_count;
		std::vector<tz::job_handle> jobs(job_count);
		for(std::size_t i = 0; i < job_count; i++)
		{
			jobs[i] = tz::job_system().execute([&images, &cube, offset = i * imgs_per_job, img_count = imgs_per_job]()
			{
				for(std::size_t j = 0; j < img_count; j++)
				{
					images[offset + j] = cube.get_image_data(offset + j);
				}
			});
		}
		for(std::size_t i = (cube.get_images().size() - remainder_imgs); i < cube.get_images().size(); i++)
		{
			images[i] = cube.get_image_data(i);
		}
		for(tz::job_handle jh : jobs)
		{
			tz::job_system().block(jh);
		}
	}
	else
	{
		TZ_PROFZONE("Load Images - Single Threaded", 0xFF44DD44);
		// if there isn't many, just do it all now.
		for(std::size_t i = 0; i < cube.get_images().size(); i++)
		{
			images[i] = cube.get_image_data(i);
		}
	}
	ret.images = images;
	return ret;
}
