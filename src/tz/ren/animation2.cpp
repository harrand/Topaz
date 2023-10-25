#include "tz/ren/animation2.hpp"
#include "tz/core/job/job.hpp"

#include "tz/gl/imported_shaders.hpp"
#include ImportedShaderHeader(animation, vertex)
#include ImportedShaderHeader(animation, fragment)

namespace tz::ren
{
//--------------------------------------------------------------------------------------------------

	animation_renderer2::animation_renderer2(info i):
	mesh_renderer2
	({
		.custom_vertex_spirv = i.custom_vertex_spirv.empty() ? ImportedShaderSource(animation, vertex) : i.custom_vertex_spirv,
		.custom_fragment_spirv = i.custom_fragment_spirv.empty() ? ImportedShaderSource(animation, fragment) : i.custom_fragment_spirv,
		.custom_options = i.custom_options,
		.texture_capacity = i.texture_capacity,
		.extra_buffers = this->evaluate_extra_buffers(i)
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
		this->gltf_load_skins(data);
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

	animation_renderer2::animated_objects_handle animation_renderer2::add_animated_objects(animated_objects_create_info info)
	{
		TZ_PROFZONE("add_gltf - add animated objects", 0xFFE54550);
		animated_object_data data
		{
			.gltf = info.gltf
		};
		// todo: populate following fields of animated_object_data:
		// - A: node_object_map (and a way to get it into the shader)
		// - B: objects
		
		// Section A

		// Section B
		// so firstly we create an empty parent object, which everyone will be a child of.
		// then we'll iterate through the gltf nodes and spawn all our subobjects.
		data.objects.push_back(mesh_renderer2::add_object
		({
			.local_transform = info.local_transform,
			.parent = info.parent
		}));
		auto gltf_hanval = static_cast<std::size_t>(static_cast<tz::hanval>(info.gltf));
		const auto& gltf = this->gltfs[gltf_hanval];
		
		auto root_nodes = gltf.data.get_root_nodes();
		for(tz::io::gltf_node node : root_nodes)
		{
			// figure out what the hell our node id is...
			this->animated_object_expand_gltf_node(data, node, std::nullopt);
		}

		// Section C
		// we need to know whether the gltf we're against has skins or not.
		// if it does, we're going to need to write into the joint buffer (extra buffer 0).

		// finally, find a slot for our animated objects, put it there and return the handle.
		auto hanval = this->animated_objects.size();
		if(this->animated_objects_free_list.size())
		{
			hanval = static_cast<std::size_t>(static_cast<tz::hanval>(this->animated_objects_free_list.front()));
			this->animated_objects_free_list.pop_front();
			this->animated_objects[hanval] = data;
		}
		else
		{
			this->animated_objects.push_back(data);
		}
		return static_cast<tz::hanval>(hanval);
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

	tz::gl::resource_handle animation_renderer2::get_joint_buffer_handle() const
	{
		tz::assert(mesh_renderer2::get_extra_buffer_count() >= 1);
		return mesh_renderer2::get_extra_buffer(0);
	}

//--------------------------------------------------------------------------------------------------

	/*static*/ std::vector<tz::gl::buffer_resource> animation_renderer2::evaluate_extra_buffers(const info& i)
	{
		std::vector<tz::gl::buffer_resource> ret = i.extra_buffers;
		// create our joint buffer.
		// what is the joint buffer?
		// so each gltf has a set of nodes. it may also have a skin.
		// a skin consists of a set of joints, which are node-ids.
		// the joint-id represents the index into this array of joints.
		// we want to have this exact list within the joint buffer for each animated object.
		// however, instead of the joint-id mapping to a node-id, it instead maps directly onto an object-id
		// this means that the shader can simply use its joint-id to index (with an offset) into this buffer to instantly get the object the joint represents.
		// at initialisation of course we have no gltfs and thus no joints.
		// we could start with a tiny buffer and just re-size whenever we add new objects, but that would be shit perf.
		// instead, we start with an initial capacity, and whenever a new animated object is added, find a region and write into it (saving the offset)
		// if we run out of space to do that *then* we can resize.
		constexpr std::size_t initial_joints_capacity = 1024;
		std::array<std::uint32_t, initial_joints_capacity> initial_joints_data;
		std::fill(initial_joints_data.begin(), initial_joints_data.end(), 0u);
		tz::gl::buffer_resource joint_buffer = tz::gl::buffer_resource::from_one(initial_joints_data);

		// joint_buffer will be extra buffer 0.
		ret.insert(ret.begin(), joint_buffer);
		return ret;
	}

//--------------------------------------------------------------------------------------------------

	void animation_renderer2::gltf_load_skins(gltf_data& gltf)
	{
		TZ_PROFZONE("add_gltf - load skins", 0xFFE54550);
		gltf.metadata.has_skins = !gltf.data.get_skins().empty();
		for(tz::io::gltf_skin skin : gltf.data.get_skins())
		{
			for(std::size_t i = 0; i < skin.joints.size(); i++)
			{
				std::uint32_t node_id = skin.joints[i];
				gltf.metadata.joint_node_map[i] = node_id;
			}
		}
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
	
	void animation_renderer2::animated_object_expand_gltf_node(animated_object_data& animated_objects, tz::io::gltf_node node, std::optional<std::size_t> parent_node_id)
	{
		TZ_PROFZONE("add animated objects - expand gltf node", 0xFFE54550);
		tz::assert(animated_objects.objects.size() >= 1, "Expanding node expected a root node to already exist, but the object list is empty. Logic error.");
		object_handle root_node = animated_objects.objects.front();
		const auto& gltf = this->gltfs[static_cast<std::size_t>(static_cast<tz::hanval>(animated_objects.gltf))];

		object_handle this_parent = tz::nullhand;
		// figure out what our parent is first.
		if(parent_node_id.has_value())
		{
			// our parent is another gltf node that was added earlier.
			// in which case we know what its object id is.
			this_parent = animated_objects.node_object_map.at(parent_node_id.value());
		}
		else
		{
			// then our parent is just the root node.
			this_parent = root_node;
		}

		// this gltf node maps to an object.
		object_handle this_object = mesh_renderer2::add_object
		({
			.local_transform = node.transform,
			.mesh = tz::nullhand,
			.parent = this_parent,
			.bound_textures = {},
		});
		// add me as a subobject to the animated objects.
		animated_objects.objects.push_back(this_object);
		// map me to this node id.
		animated_objects.node_object_map[node.id] = this_object;
		// todo: label us with a name?

		if(node.mesh != static_cast<std::size_t>(-1))
		{
			// this node has a mesh attached.
			// the mesh may have multiple submeshes.
			// for each submesh, create a new child object, set that mesh to the submesh and make sure we're its parent.
			// remember, this is only a workaround for not supporting multiple submeshes per object - the child object should have the exact same transforms etc...
			const std::size_t submesh_count = gltf.data.get_meshes()[node.mesh].submeshes.size();
			// this is where the metadata comes in handy!
			const std::size_t submesh_offset = gltf.metadata.mesh_submesh_indices[node.mesh];
			for(std::size_t i = submesh_offset; i < (submesh_offset + submesh_count); i++)
			{
				std::vector<impl::texture_locator> bound_textures = {};
				if(gltf.metadata.submesh_materials[i].has_value())
				{
					const tz::io::gltf_material& mat = gltf.metadata.submesh_materials[i].value();
					tz::assert(gltf.textures.size() > mat.color_texture_id, "GLTF node material specifies texture id %zu, which does not seem to exist within the gltf", mat.color_texture_id);
					tz::assert(mat.color_texcoord_id == 0, "animation_renderer2 only supports one texcoord for all properties of a material. Albedo property wants a texcoord of id %zu", mat.color_texcoord_id);
					tz::assert(bound_textures.size() == 0);
					// this is texture binding 0. albedo.
					bound_textures.push_back(impl::texture_locator
					{
						.colour_tint = mat.color_factor.swizzle<0, 1, 2>(),
						.texture = gltf.textures[mat.color_texture_id]
					});
					// is there a metallic roughness texture?
					if(mat.metallic_roughness_texture_id != static_cast<std::size_t>(-1))
					{
						// yes.
						// this is texture binding 1. metallic roughness.
						tz::assert(mat.metallic_roughness_texcoord_id == 0, "animation_renderer2 only supports one texcoord for all properties of a material. Metallic roughness property wants a texcoord of id %zu", mat.metallic_roughness_texcoord_id);
						tz::assert(bound_textures.size() == 1);
						bound_textures.push_back(impl::texture_locator
						{
							.colour_tint = tz::vec3{0.0f, mat.roughness_factor, mat.metallic_factor},
							.texture = gltf.textures[mat.metallic_roughness_texture_id]
						});
					}
				}
				object_handle child = mesh_renderer2::add_object
				({
					.local_transform = {},
					.mesh = gltf.meshes[i],
					.parent = this_object,
					.bound_textures = bound_textures,
				});
				// todo: give this sub-subobject a name too if you want?
				animated_objects.objects.push_back(child);
			}
		}

		for(std::size_t child_idx : node.children)
		{
			this->animated_object_expand_gltf_node(animated_objects, gltf.data.get_nodes()[child_idx], node.id);
		}
	}
}