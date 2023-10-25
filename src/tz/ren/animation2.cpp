#include "tz/ren/animation2.hpp"
#include "tz/core/job/job.hpp"

namespace tz::ren
{
//--------------------------------------------------------------------------------------------------

	animation_renderer2::animation_renderer2(info i):
	mesh_renderer2
	({
		.custom_vertex_spirv = i.custom_vertex_spirv,
		.custom_fragment_spirv = i.custom_fragment_spirv,
		.custom_options = i.custom_options,
		.texture_capacity = i.texture_capacity
	})
	{

	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::update(float delta)
	{
		TZ_PROFZONE("animation_renderer2 - update", 0xFFC52530);
		mesh_renderer2::update();
	}

//--------------------------------------------------------------------------------------------------

	animation_renderer2::gltf_handle animation_renderer2::add_gltf(tz::io::gltf gltf)
	{
		TZ_PROFZONE("animation_renderer2 - add gltf", 0xFFE54550);
		// first we load the meshes and textures that the gltf comes with.
		gltf_data data
		{
			.data = gltf,
			// this->gltf_load_XYZ(this) fills this for us, leave it empty for now.
			.meshes = {},
			.textures = {}
		};
		this->gltf_load_meshes(data);
		this->gltf_load_textures(data);
		// then we create the gltf data, put it in a free slot, and return the handle.

		std::size_t hanval = this->gltfs.size();
		if(this->gltf_free_list.size())
		{
			hanval = static_cast<std::size_t>(static_cast<tz::hanval>(this->gltf_free_list.front()));
			this->gltf_free_list.pop_front();
			this->gltfs[hanval] = data;
		}
		else
		{
			this->gltfs.push_back(data);
		}
		return static_cast<tz::hanval>(hanval);
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::remove_gltf(gltf_handle handle)
	{
		TZ_PROFZONE("animation_renderer2 - remove gltf", 0xFFE54550);
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
		tz::assert(!this->gltf_is_in_free_list(handle), "Double-free on gltf %zu - was already in free-list", hanval);
		// todo: remove all animated objects that use this gltf. if the gltf goes, gonna crash if animated objects are trying to use it.
		this->gltfs[hanval] = gltf_data{};
		this->gltf_free_list.push_back(handle);
	}

//--------------------------------------------------------------------------------------------------

	bool animation_renderer2::gltf_is_in_free_list(gltf_handle handle) const
	{
		return std::find(this->gltf_free_list.begin(), this->gltf_free_list.end(), handle) != this->gltf_free_list.end();
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::animation_advance(float delta)
	{
		TZ_PROFZONE("animation_renderer2 - animation advance", 0xFFE54550);

	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::gltf_load_meshes(gltf_data& gltf)
	{
		TZ_PROFZONE("add_gltf - load meshes", 0xFFE54550);
		tz::assert(gltf.meshes.empty(), "gltf_load_meshes(gltf_data) invoked on parameter that already has meshes. It should be empty! Logic error.");
		std::size_t gltf_submesh_total = 0;
		for(std::size_t i = 0; i < gltf.data.get_meshes().size(); i++)
		{
			TZ_PROFZONE("load - add gltf Mesh", 0xFFE54550);
			std::size_t submesh_count = gltf.data.get_meshes()[i].submeshes.size();
			gltf.metadata.mesh_submesh_indices.push_back(gltf_submesh_total);
			gltf_submesh_total += submesh_count;
			for(std::size_t j = 0; j < submesh_count; j++)
			{
				mesh_renderer2::mesh submesh;

				std::size_t material_id = gltf.data.get_meshes()[i].submeshes[j].material_id;
				auto& maybe_material = gltf.metadata.submesh_materials.emplace_back();
				if(material_id != static_cast<std::size_t>(-1))
				{
					// no material bound, so no textures bound.
					maybe_material = gltf.data.get_materials()[material_id];
				}
				tz::io::gltf_submesh_data gltf_submesh = gltf.data.get_submesh_vertex_data(i, j);
				// copy over indices.
				submesh.indices.resize(gltf_submesh.indices.size());
				std::transform(gltf_submesh.indices.begin(), gltf_submesh.indices.end(), submesh.indices.begin(),
				[](std::uint32_t idx)-> impl::mesh_index{return idx;});
				// copy over vertices.
				std::transform(gltf_submesh.vertices.begin(), gltf_submesh.vertices.end(), std::back_inserter(submesh.vertices),
				[](tz::io::gltf_vertex_data vtx)-> impl::mesh_vertex
				{
					impl::mesh_vertex ret;

					// these are easy.
					ret.position = vtx.position;
					ret.normal = vtx.normal;
					ret.tangent = vtx.tangent;

					// remember we only support one texcoord
					ret.texcoord = vtx.texcoordn.front();

					// note: the joint indices are in terms of the gltf skin's list of joints.
					// this is troublesome because the shader deals with objects and object-ids, but has no idea about the gltf data.
					// so we just write them as-is for now. we will eventually go on to invoke `resource_write_joint_indices` which
					// will perform resource writes to correct this data. we can't do it now as we dont have the joint->node and node->object maps
					// ready yet.
					ret.joint_indices = vtx.jointn.front();
					ret.joint_weights = vtx.weightn.front();
					// ignore colours. could theoretically incorporate that into tints, but will be very difficult to translate to texture locator tints properly.
					return ret;
				});
				// add the mesh!
				gltf.meshes.push_back(mesh_renderer2::add_mesh(submesh));
			}
		}
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::gltf_load_textures(gltf_data& gltf)
	{
		TZ_PROFZONE("add_gltf - load textures", 0xFFE54550);
		std::vector<tz::io::image> images;
		images.resize(gltf.data.get_images().size());
		if(gltf.data.get_images().size() > 8)
		{
			TZ_PROFZONE("load textures - execute jobs", 0xFF44DD44);
			// we should split this into threads.
			std::size_t job_count = std::thread::hardware_concurrency();
			std::size_t imgs_per_job = gltf.data.get_images().size() / job_count; 
			std::size_t remainder_imgs = gltf.data.get_images().size() % job_count;
			std::vector<tz::job_handle> jobs(job_count);
			for(std::size_t i = 0; i < job_count; i++)
			{
				jobs[i] = tz::job_system().execute([&images, &gltf, offset = i * imgs_per_job, img_count = imgs_per_job]()
				{
					for(std::size_t j = 0; j < img_count; j++)
					{
						images[offset + j] = gltf.data.get_image_data(offset + j);
					}
				});
			}
			for(std::size_t i = (gltf.data.get_images().size() - remainder_imgs); i < gltf.data.get_images().size(); i++)
			{
				images[i] = gltf.data.get_image_data(i);
			}
			for(tz::job_handle jh : jobs)
			{
				tz::job_system().block(jh);
			}
		}
		else
		{
			TZ_PROFZONE("load textures - single-threaded brute force", 0xFF44DD44);
			// if there isn't many, just do it all now.
			for(std::size_t i = 0; i < gltf.data.get_images().size(); i++)
			{
				images[i] = gltf.data.get_image_data(i);
			}
		}
		for(const tz::io::image& img : images)
		{
			gltf.textures.push_back(mesh_renderer2::add_texture(img));
		}
	}

//--------------------------------------------------------------------------------------------------
}