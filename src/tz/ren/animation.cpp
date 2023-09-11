#include "tz/ren/animation.hpp"
#include "imgui.h"
#include "tz/core/job/job.hpp"
#include "tz/gl/imported_shaders.hpp"

#include ImportedShaderHeader(animation, vertex)
#include ImportedShaderHeader(animation, fragment)

namespace tz::ren
{
	animation_renderer::animation_renderer(unsigned int total_textures):
	mesh_renderer(total_textures, ImportedShaderSource(animation, vertex), ImportedShaderSource(animation, fragment))
	{
	}

	void animation_renderer::dbgui()
	{
		if(ImGui::BeginTabBar("#1234"))
		{
			mesh_renderer::dbgui_tab_overview();
			mesh_renderer::dbgui_tab_render();
			this->dbgui_tab_animation();
			ImGui::EndTabBar();
		}
	}

	void animation_renderer::update(float delta)
	{
		animation_renderer::update();
		for(std::size_t i = 0; i < draw_count(); i++)
		{
			this->object_extras[i].is_animated = false;
		}
		this->animation_advance(delta);
	}

	animation_renderer::object_handle animation_renderer::add_object(object_init_data init)
	{
		auto handle = mesh_renderer::add_object(init);
		tz::assert(static_cast<std::size_t>(static_cast<tz::hanval>(handle)) == this->object_extras.size());
		this->object_extras.push_back
		({
			.base_transform = init.trs
		});
		return handle;
	}

	std::string_view animation_renderer::get_object_name(object_handle h) const
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(h));
		tz::assert(hanval < this->object_extras.size());
		return this->object_extras[hanval].name;
	}

	std::vector<animation_renderer::object_handle> animation_renderer::find_objects_by_name(const char* name) const
	{
		std::vector<object_handle> ret = {};
		for(std::size_t i = 0; i < this->object_extras.size(); i++)
		{
			if(this->object_extras[i].name == name)
			{
				ret.push_back(static_cast<tz::hanval>(i));
			}
		}
		return ret;
	}

	tz::trs animation_renderer::get_object_base_transform(object_handle h) const
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(h));
		return this->object_extras[hanval].base_transform;
	}

	void animation_renderer::set_object_base_transform(object_handle h, tz::trs local_transform)
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(h));
		this->object_extras[hanval].base_transform = local_transform;	
	}

	animation_renderer::asset_package animation_renderer::add_gltf(tz::io::gltf gltf)
	{
		return this->add_gltf(gltf, override_package{});
	}

	animation_renderer::asset_package animation_renderer::add_gltf(tz::io::gltf gltf, override_package opkg)
	{
		return this->add_gltf(gltf, tz::nullhand, opkg);
	}

	animation_renderer::asset_package animation_renderer::add_gltf(tz::io::gltf gltf, object_handle parent)
	{
		return this->add_gltf(gltf, parent, override_package{});
	}

	animation_renderer::asset_package animation_renderer::add_gltf(tz::io::gltf gltf, object_handle parent, override_package opkg)
	{
		// maintain offsets so we can support multiple gltfs.
		// add the new gltf.
		this->gltfs.push_back
		({
			.data = gltf,
			.object_offset = static_cast<unsigned int>(mesh_renderer::draw_count()),
		});
		auto& this_gltf = this->gltfs.back();
		this_gltf.assets.gltfh = static_cast<tz::hanval>(this->gltfs.size() - 1);

		// skins
		this->node_handle_skins(this_gltf);
		// meshes
		std::vector<mesh_handle> meshes;
		if(opkg.overrides.contains(override_flag::mesh))
		{
			meshes = opkg.pkg.meshes;
			this->shallow_patch_meshes(this_gltf);
		}
		else
		{
			meshes = this->node_handle_meshes(this_gltf);
		}
		for(mesh_handle mesh : meshes)
		{
			this_gltf.assets.meshes.push_back(mesh);
		}
		// textures
		std::vector<texture_handle> materials;
		if(opkg.overrides.contains(override_flag::texture))
		{
			materials = opkg.pkg.textures;
		}
		else
		{
			materials = this->node_handle_materials(this_gltf);
		}
		for(texture_handle texture : materials)
		{
			this_gltf.assets.textures.push_back(texture);
		}

		// objects
		// expand nodes recursively.
		auto gltf_nodes = gltf.get_root_nodes();
		auto all_nodes = gltf.get_nodes();
		for(tz::io::gltf_node node : gltf_nodes)
		{
			// we deal with node indices, so yeah... we gotta recalculate them (boo!)
			auto iter = std::find(all_nodes.begin(), all_nodes.end(), node);
			tz::assert(iter != all_nodes.end());
			std::size_t node_id = std::distance(all_nodes.begin(), iter);
			tz::assert(node.id == node_id);

			this->expand_current_gltf_node(this_gltf, node_id, std::nullopt, parent);
		}

		this->write_inverse_bind_matrices(this_gltf);

		std::optional<tz::vec2ui32> maybe_joint_span = std::nullopt;
		if(this_gltf.data.get_skins().size())
		{
			maybe_joint_span = this->write_skin_object_data(this_gltf);
		}
		//this->resource_write_joint_indices(this_gltf);
		if(maybe_joint_span.has_value())
		{
			for(object_handle oh : this_gltf.assets.objects)
			{
				mesh_renderer::get_object_data(oh).extra_indices = maybe_joint_span.value().with_more(0u).with_more(0u);
			}
		}

		return this_gltf.assets;
	}

	std::size_t animation_renderer::get_animation_count(const asset_package& pkg) const
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		return this->gltfs[hanval].data.get_animations().size();
	}

	std::optional<std::size_t> animation_renderer::get_playing_animation(const asset_package& pkg) const
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		return this->gltfs[hanval].playback.playing_animation_id;
	}

	std::string_view animation_renderer::get_animation_name(const asset_package& pkg, std::size_t animation_id) const
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		return this->gltfs[hanval].data.get_animations()[animation_id].name;
	}

	void animation_renderer::play_animation(const asset_package& pkg, std::size_t animation_id, bool loop)
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		this->gltfs[hanval].playback.playing_animation_id = animation_id;
		this->gltfs[hanval].playback.time = 0.0f;
		this->gltfs[hanval].playback.loop = loop;
	}

	void animation_renderer::queue_animation(const asset_package& pkg, std::size_t animation_id, bool loop)
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		this->gltfs[hanval].playback.queued_animations.push({.id = animation_id, .loop = loop});
	}

	void animation_renderer::skip_animation(const asset_package& pkg)
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		auto& playback = this->gltfs[hanval].playback;
		if(playback.queued_animations.size())
		{
			playback.playing_animation_id = playback.queued_animations.front().id;
			playback.loop = playback.queued_animations.front().loop;
			playback.time = 0.0f;
			playback.queued_animations.pop();
		}
		else
		{
			playback.playing_animation_id = std::nullopt;
		}
	}

	void animation_renderer::halt_animation(const asset_package& pkg)
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		this->gltfs[hanval].playback.playing_animation_id = std::nullopt;
		this->gltfs[hanval].playback.time = 0.0f;
	}

	float animation_renderer::get_animation_speed(const asset_package& pkg) const
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		return this->gltfs[hanval].playback.time_warp;
	}

	void animation_renderer::set_animation_speed(const asset_package& pkg, float speed)
	{
		auto hanval = static_cast<std::size_t>(static_cast<tz::hanval>(pkg.gltfh));
		tz::assert(this->gltfs.size() > hanval);
		this->gltfs[hanval].playback.time_warp = speed;
	}

	void animation_renderer::update()
	{
		for(std::size_t i = 0; i < mesh_renderer::draw_count(); i++)
		{
			auto maybe_tree_id = mesh_renderer::object_tree.find_node(i);
			tz::assert(maybe_tree_id.has_value());
			auto& tree_node = mesh_renderer::object_tree.get_node(maybe_tree_id.value());
			auto& extra = this->object_extras[i];
			if(extra.is_animated)
			{
				tree_node.local_transform = extra.animation_trs_offset;
			}
			else
			{
				tree_node.local_transform = extra.base_transform;
			}
			extra.animation_trs_offset = extra.base_transform;
		}
		mesh_renderer::update();
	}

	void animation_renderer::expand_current_gltf_node(gltf_info& gltf, std::size_t node_id, std::optional<std::size_t> parent_node_id, object_handle parent_override)
	{
		const tz::io::gltf_node& node = gltf.data.get_nodes()[node_id];
		object_handle this_object = tz::nullhand;
		if(node.skin != static_cast<std::size_t>(-1))
		{
			// todo: skin processing.
		}
		object_handle parent = parent_override;
		if(parent == tz::nullhand)
		{
			if(parent_node_id.has_value())
			{
				parent = gltf.node_object_map.at(parent_node_id.value());
			}
		}
		std::size_t this_extra_id = this->object_extras.size();
		this_object = this->add_object
		({
			.trs = node.transform,
			.mesh = {},
			.bound_textures = {},
			.parent = parent
		});
		// new object belongs to this asset package.
		gltf.assets.objects.push_back(this_object);
		// node id also maps to this object.
		gltf.node_object_map[node_id] = this_object;
		// set the name to whatever the gltf node had.
		this->object_extras[this_extra_id].name = node.name;

		// TODO: one object can only render one mesh at a time.
		// a mesh corresponds to a single gltf submesh, but a gltf node can correspond to a gltf mesh i.e multiple submeshes
		// in this case, we hit trouble because an object can only correspond to a single mesh, but we want it to correspond to multiple.
		// what we do is simple:
		// add the initial object with no mesh attached.
		// for each submesh that needs to be attached, create a new zero-TRS child object of the original, and give each of them a submesh.
		// remember that none of this is implemented yet. this is what you should do.
		if(node.mesh != static_cast<std::size_t>(-1))
		{
			// make a new object for each submesh, their parents should be this_object
			std::size_t submesh_count = gltf.data.get_meshes()[node.mesh].submeshes.size();
			std::size_t submesh_offset = gltf.metadata.mesh_submesh_indices[node.mesh];
			this->object_extras[this_extra_id].submesh_count = submesh_count;
			for(std::size_t i = submesh_offset; i < (submesh_offset + submesh_count); i++)
			{
				std::array<texture_locator, mesh_renderer_max_tex_count> bound_textures = {};
				if(gltf.metadata.submesh_materials[i].has_value())
				{
					bound_textures[0] = texture_locator
					{
						.colour_tint = tz::vec3::filled(1.0f),
						.texture = gltf.assets.textures[gltf.metadata.submesh_materials[i]->color_texture_id]
					};
				}
				object_handle child = this->add_object
				({
					.trs = {},
					.mesh = gltf.assets.meshes[i],
					.bound_textures = {bound_textures},
					.parent = this_object
				});
				this->object_extras.back().name = "Submesh " + std::to_string(i - submesh_offset) + std::string{" - "} + this->object_extras[this_extra_id].name;
				// Reminder: Sanity check. this means that `gltf_node_id == object_id if we are first gltf` is no longer true.
				gltf.assets.objects.push_back(child);
			}
		}

		for(std::size_t child_idx : node.children)
		{
			this->expand_current_gltf_node(gltf, child_idx, node_id);
		}
	}

	void animation_renderer::node_handle_skins(gltf_info& gltf_info)
	{
		gltf_info.metadata.has_skins = !gltf_info.data.get_skins().empty();
		for(tz::io::gltf_skin skin : gltf_info.data.get_skins())
		{
			for(std::size_t i = 0; i < skin.joints.size(); i++)
			{
				std::uint32_t node_id = skin.joints[i];
				gltf_info.metadata.joint_node_map[i] = node_id;
			}
		}
	}

	tz::vec2ui32 animation_renderer::write_skin_object_data(gltf_info& gltf_info)
	{
		constexpr std::size_t joint_buffer_id = 0;
		tz::gl::resource_handle joint_bufferh = mesh_renderer::get_extra_buffer_handle(joint_buffer_id);
		tz::gl::RendererEditBuilder edit;
		tz::assert(gltf_info.data.get_skins().size(), "dont call write_skin_object_data on a gltf without skins!");
		const auto& joints = gltf_info.data.get_skins().front().joints;
		// if id == 0, then we havent written into the buffer yet, so its pseudo-empty. that means we resize to what we want and completely rewrite its contents.
		// whats the current size?
		auto sz = mesh_renderer::render_pass_get_resource(joint_bufferh).data().size_bytes();	
		sz = std::max(sz, sizeof(std::uint32_t));
		std::size_t new_data_size = sizeof(std::uint32_t) * joints.size();
		std::vector<std::uint32_t> new_data;
		new_data.resize(joints.size());
		for(std::size_t i = 0; i < joints.size(); i++)
		{
			const std::size_t joint_index = i;
			const std::size_t node_id = gltf_info.metadata.joint_node_map.at(joint_index);
			object_handle objecth = gltf_info.node_object_map.at(node_id);
			auto object_id = static_cast<std::size_t>(static_cast<tz::hanval>(objecth));
			new_data[i] = object_id;
		}
		std::span<const std::uint32_t> new_data_view{new_data};
		edit.buffer_resize
		({
			.buffer_handle = joint_bufferh,
			.size = sz + new_data_size	
		});
		edit.write
		({
			.resource = joint_bufferh,
			.data = std::as_bytes(new_data_view),
			.offset = sz
		});
		mesh_renderer::render_pass_edit(edit);
		return static_cast<tz::vec2ui32>(tz::vector<std::size_t, 2>{sz / sizeof(std::uint32_t), joints.size()});
	}

	void animation_renderer::write_inverse_bind_matrices(gltf_info& gltf_info)
	{
		for(tz::io::gltf_skin skin : gltf_info.data.get_skins())
		{
			for(std::size_t i = 0; i < skin.joints.size(); i++)
			{
				std::uint32_t node_id = skin.joints[i];
				object_handle handle = gltf_info.node_object_map.at(node_id);
				object_data& obj = mesh_renderer::get_object_data(handle);
				// extra = inverse bind matrix
				obj.extra = skin.inverse_bind_matrices[i];
			}
		}
	}

	void animation_renderer::resource_write_joint_indices(gltf_info& gltf_info)
	{
		std::deque<std::vector<vertex_t>> amended_vertex_storage;
		// we wrote some joint indices for each vertex earlier.
		// however, we need to amend them.
		tz::gl::RendererEditBuilder builder;
		// we only write to the meshes that belong to this gltf!
		for(mesh_handle m : gltf_info.assets.meshes)
		{
			const auto& mloc = mesh_renderer::get_mesh_locator(m);
			// read the joint indices from resource data.		
			std::span<const vertex_t> initial_vertices = mesh_renderer::read_vertices(mloc);
			// copy into a new buffer.
			auto& amended_vertices = amended_vertex_storage.emplace_back(initial_vertices.size());
			std::copy(initial_vertices.begin(), initial_vertices.end(), amended_vertices.begin());
			if(!gltf_info.metadata.has_skins)
			{
				// there aren't any joints! skip!
				continue;
			}
			// make the joint index changes.
			for(vertex_t& vtx : amended_vertices)
			{
				for(tz::vec4ui32& joint_index_cluster : vtx.joint_indices)
				{
					for(std::size_t i = 0; i < 4; i++)
					{
						// get joint index
						std::uint32_t joint_index = joint_index_cluster[i];
						// convert directly to object id.
						std::size_t gltf_node_id = gltf_info.metadata.joint_node_map.at(joint_index);
						object_handle handle = gltf_info.node_object_map.at(gltf_node_id);
						joint_index_cluster[i] = static_cast<std::size_t>(static_cast<tz::hanval>(handle));
					}
				}
			}
			std::span<const vertex_t> amended_span = amended_vertices;
			// queue a resource write with the changes.
			builder.write
			({
				.resource = mesh_renderer::render_pass_get_vertex_buffer_handle(),
				.data = std::as_bytes(amended_span),
				.offset = sizeof(vertex_t) * mloc.vertex_offset,
			});
		}
		mesh_renderer::render_pass_edit(builder);
	}

	void animation_renderer::animation_advance(float delta)
	{
		for(auto& gltf : this->gltfs)
		{
			if(gltf.data.get_animations().size())
			{
				gltf.playback.time += (delta * gltf.playback.time_warp);
				// only do first animation.
				if(!gltf.playback.playing_animation_id.has_value())
				{
					continue;
				}
				const auto& anim = gltf.data.get_animations()[gltf.playback.playing_animation_id.value()];
				// loop animations.
				if(gltf.playback.time > anim.max_time)
				{
					if(gltf.playback.loop)
					{
						if(gltf.playback.time_warp >= 0.0f)
						{
							gltf.playback.time = 0.0f;
						}
						else
						{
							gltf.playback.time = anim.max_time;
						}
					}
					else
					{
						// time has run out. is another one queued?
						if(gltf.playback.queued_animations.size())
						{
							// set that as a the new playback and then exit.
							std::size_t anim_id = gltf.playback.queued_animations.front().id;
							gltf.playback.loop = gltf.playback.queued_animations.front().loop;
							gltf.playback.queued_animations.pop();
							gltf.playback.playing_animation_id = anim_id;
							gltf.playback.time = 0.0f;
						}
					}
				}
				if(gltf.playback.time < 0.0f)
				{
					gltf.playback.time = anim.max_time;
				}

				// update nodes.
				for(std::size_t nid = 0; nid < anim.node_animation_data.size(); nid++)
				{
					object_handle objh = gltf.node_object_map[nid];
					auto& extra = this->object_extras[static_cast<std::size_t>(static_cast<tz::hanval>(objh))];
					extra.is_animated = true;
					
					// actual animation stuff now.
					const auto& [kf_positions, kf_rotations, kf_scales] = anim.node_animation_data[nid];
					auto [pos_before_id, pos_after_id] = gltf.interpolate_animation_keyframes(kf_positions.begin(), kf_positions.end());
					auto [rot_before_id, rot_after_id] = gltf.interpolate_animation_keyframes(kf_rotations.begin(), kf_rotations.end());
					auto [scale_before_id, scale_after_id] = gltf.interpolate_animation_keyframes(kf_scales.begin(), kf_scales.end());
					if(kf_positions.size() > 1)
					{
						auto before = kf_positions.begin();
						auto after = before;
						std::advance(before, pos_before_id);
						std::advance(after, pos_after_id);
						//tz::assert(gltf.playback.time >= before->time_point);
						float pos_interp = std::clamp((gltf.playback.time - before->time_point) / (after->time_point - before->time_point), 0.0f, 1.0f);
						tz::vec3 beforet = before->transform.swizzle<0, 1, 2>();
						tz::vec3 aftert = after->transform.swizzle<0, 1, 2>();
						extra.animation_trs_offset.translate = beforet + ((aftert - beforet) * pos_interp);
					}

					if(kf_rotations.size() > 1)
					{
						auto before = kf_rotations.begin();
						auto after = before;
						std::advance(before, rot_before_id);
						std::advance(after, rot_after_id);
						//tz::assert(gltf.playback.time >= before->time_point);
						float rot_interp = std::clamp((gltf.playback.time - before->time_point) / (after->time_point - before->time_point), 0.0f, 1.0f);
						tz::quat beforer = before->transform.normalised();
						tz::quat afterr = after->transform.normalised();
						extra.animation_trs_offset.rotate = beforer.slerp(afterr, rot_interp);
					}

					if(kf_scales.size() > 1)
					{
						auto before = kf_scales.begin();
						auto after = before;
						std::advance(before, scale_before_id);
						std::advance(after, scale_after_id);
						//tz::assert(gltf.playback.time >= before->time_point);
						float scale_interp = std::clamp((gltf.playback.time - before->time_point) / (after->time_point - before->time_point), 0.0f, 1.0f);
						tz::vec3 befores = before->transform.swizzle<0, 1, 2>();
						tz::vec3 afters = after->transform.swizzle<0, 1, 2>();
						extra.animation_trs_offset.scale = befores + ((afters - befores) * scale_interp);
					}
				}
			}
		}	
	}

	void animation_renderer::shallow_patch_meshes(gltf_info& gltf_info)
	{
		const tz::io::gltf& gltf = gltf_info.data;
		std::size_t gltf_submesh_total = 0;
		for(std::size_t i = 0; i < gltf.get_meshes().size(); i++)
		{
			TZ_PROFZONE("Anim Renderer - Shallow Patch GLTF Mesh", 0xFF44DD44);
			std::size_t submesh_count = gltf.get_meshes()[i].submeshes.size();
			gltf_info.metadata.mesh_submesh_indices.push_back(gltf_submesh_total);	
			for(std::size_t j = 0; j < submesh_count; j++)
			{
				// pretend `existing_meshes[j]` is where we just loaded this mesh into.
				std::size_t material_id = gltf.get_meshes()[i].submeshes[j].material_id;
				auto& maybe_material = gltf_info.metadata.submesh_materials.emplace_back();
				if(material_id != static_cast<std::size_t>(-1))
				{
					maybe_material = gltf.get_materials()[material_id];
				}
			}
			gltf_submesh_total += submesh_count;
		}
	}

	std::vector<animation_renderer::mesh_handle> animation_renderer::node_handle_meshes(gltf_info& gltf_info)
	{
		const tz::io::gltf& gltf = gltf_info.data;
		std::vector<animation_renderer::mesh_handle> ret;
		std::size_t gltf_submesh_total = 0;
		for(std::size_t i = 0; i < gltf.get_meshes().size(); i++)
		{
			TZ_PROFZONE("Anim Renderer - Add GLTF Mesh", 0xFF44DD44);
			std::size_t submesh_count = gltf.get_meshes()[i].submeshes.size();
			gltf_info.metadata.mesh_submesh_indices.push_back(gltf_submesh_total);
			gltf_submesh_total += submesh_count;
			for(std::size_t j = 0; j < submesh_count; j++)
			{
				mesh_t submesh;

				std::size_t material_id = gltf.get_meshes()[i].submeshes[j].material_id;
				auto& maybe_material = gltf_info.metadata.submesh_materials.emplace_back();
				if(material_id != static_cast<std::size_t>(-1))
				{
					// no material bound, so no textures bound.
					maybe_material = gltf.get_materials()[material_id];
				}
				tz::io::gltf_submesh_data gltf_submesh = gltf.get_submesh_vertex_data(i, j);
				// copy over indices.
				submesh.indices.resize(gltf_submesh.indices.size());
				std::transform(gltf_submesh.indices.begin(), gltf_submesh.indices.end(), submesh.indices.begin(),
				[](std::uint32_t idx)-> index{return idx;});
				// copy over vertices.
				std::transform(gltf_submesh.vertices.begin(), gltf_submesh.vertices.end(), std::back_inserter(submesh.vertices),
				[](tz::io::gltf_vertex_data vtx)-> vertex_t
				{
					vertex_t ret;

					// these are easy.
					ret.position = vtx.position;
					ret.normal = vtx.normal;
					ret.tangent = vtx.tangent;

					// texcoord a little more troublesome!
					constexpr std::size_t texcoord_count = std::min(static_cast<int>(mesh_renderer_max_tex_count), tz::io::gltf_max_texcoord_attribs);
					for(std::size_t i = 0; i < texcoord_count; i++)
					{
						ret.texcoordn[i] = vtx.texcoordn[i].with_more(0.0f).with_more(0.0f);
					}

					// similar with weights.
					constexpr std::size_t weight_count = std::min(static_cast<int>(mesh_renderer_max_weight_count / 4), tz::io::gltf_max_joint_attribs);
					// note: the joint indices are in terms of the gltf skin's list of joints.
					// this is troublesome because the shader deals with objects and object-ids, but has no idea about the gltf data.
					// so we just write them as-is for now. we will eventually go on to invoke `resource_write_joint_indices` which
					// will perform resource writes to correct this data. we can't do it now as we dont have the joint->node and node->object maps
					// ready yet.
					for(std::size_t i = 0; i < weight_count; i++)
					{
						ret.joint_indices[i] = vtx.jointn[i];
						ret.joint_weights[i] = vtx.weightn[i];
					}
					// ignore colours. could theoretically incorporate that into tints, but will be very difficult to translate to texture locator tints properly.
					return ret;
				});
				// add the mesh!
				ret.push_back(mesh_renderer::add_mesh(submesh));
			}
		}
		return ret;
	}

	std::vector<animation_renderer::texture_handle> animation_renderer::node_handle_materials(gltf_info& gltf_info)
	{
		std::vector<animation_renderer::texture_handle> ret;
		const tz::io::gltf& gltf = gltf_info.data;
		std::vector<tz::io::image> images;
		images.resize(gltf.get_images().size());
		if(gltf.get_images().size() > 8)
		{
			TZ_PROFZONE("Load Images - Jobs", 0xFF44DD44);
			// we should split this into threads.
			std::size_t job_count = std::thread::hardware_concurrency();
			std::size_t imgs_per_job = gltf.get_images().size() / job_count; 
			std::size_t remainder_imgs = gltf.get_images().size() % job_count;
			std::vector<tz::job_handle> jobs(job_count);
			for(std::size_t i = 0; i < job_count; i++)
			{
				jobs[i] = tz::job_system().execute([&images, &gltf, offset = i * imgs_per_job, img_count = imgs_per_job]()
				{
					for(std::size_t j = 0; j < img_count; j++)
					{
						images[offset + j] = gltf.get_image_data(offset + j);
					}
				});
			}
			for(std::size_t i = (gltf.get_images().size() - remainder_imgs); i < gltf.get_images().size(); i++)
			{
				images[i] = gltf.get_image_data(i);
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
			for(std::size_t i = 0; i < gltf.get_images().size(); i++)
			{
				images[i] = gltf.get_image_data(i);
			}
		}
		for(const tz::io::image& img : images)
		{
			ret.push_back(mesh_renderer::add_texture(tz::vec2ui{img.width, img.height}, img.data));
		}
		return ret;
	}

	void animation_renderer::dbgui_tab_animation()
	{
		if(ImGui::BeginTabItem("Animation"))
		{
			ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "ANIMATION DATA");
			ImGui::Spacing();
			static bool display_trs = false;
			ImGui::Checkbox("Display Node TRS", &display_trs);
			mesh_renderer::object_tree.dbgui(display_trs);

			static int animation_id = 0;
			constexpr float slider_height = 160.0f;
			if(this->gltfs.size() && ImGui::CollapsingHeader("Animation Playback", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::VSliderInt("##animid", ImVec2{18.0f, slider_height}, &animation_id, 0, this->gltfs.size() - 1);
				auto& anim = this->gltfs[animation_id];
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
				if(ImGui::BeginChild("#who_asked", ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
				{
					int anim_cursor = anim.playback.playing_animation_id.value_or(-1);
					ImGui::Checkbox("Loop", &anim.playback.loop);
					if(ImGui::RadioButton("No Animation", !anim.playback.playing_animation_id.has_value()))
					{
						anim.playback.playing_animation_id = std::nullopt;
						anim.playback.time = 0.0f;
					}
					for(std::size_t i = 0; i < anim.data.get_animations().size(); i++)
					{
						if(ImGui::RadioButton(anim.data.get_animations()[i].name.c_str(), &anim_cursor, i))
						{
							anim.playback.playing_animation_id = anim_cursor;
							anim.playback.time = 0.0f;
						}
					}
					if(anim.playback.playing_animation_id.has_value())
					{
						const auto& current_anim = anim.data.get_animations()[anim.playback.playing_animation_id.value()];
						ImGui::Text("%s", current_anim.name.c_str());
						const float time_max = current_anim.max_time;
						ImGui::Text("%.2f/%.2f", anim.playback.time, time_max);
						ImGui::SliderFloat("Time Warp", &anim.playback.time_warp, -5.0f, 5.0f);
						ImGui::ProgressBar(anim.playback.time / time_max);
					}
				}
				ImGui::EndChild();
			}
			static int object_id = 0;
			if(mesh_renderer::draw_count() > 0 && ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				constexpr float slider_height = 160.0f;
				ImGui::VSliderInt("##objectid", ImVec2{18.0f, slider_height}, &object_id, 0, mesh_renderer::draw_count() - 1);
				std::string objname = "Object " + std::to_string(object_id);

				ImGui::SameLine();

				// add slight horizontal spacing between slider and child.
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
				object_extra_info& extra = this->object_extras[object_id];
				if(ImGui::BeginChild(objname.c_str(), ImVec2(0, slider_height), false, ImGuiWindowFlags_ChildWindow))
				{
					ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, extra.name.c_str());
					if(ImGui::TreeNode("Base Transform"))
					{
						extra.base_transform.dbgui();
						ImGui::TreePop();
					}
					if(ImGui::TreeNode("Animation Offset"))
					{
						extra.animation_trs_offset.dbgui();
						ImGui::TreePop();
					}
					ImGui::Text("Child Submesh Count: %zu", extra.submesh_count);
				}
				ImGui::EndChild();
			}
			ImGui::EndTabItem();
		}
	}

	std::pair<std::size_t, std::size_t> animation_renderer::gltf_info::interpolate_animation_keyframes(keyframe_iterator front, keyframe_iterator back) const
	{
		keyframe_iterator iter = front;
		while(iter != back && iter->time_point <= this->playback.time)
		{
			iter++;
		}
		std::size_t idx = std::distance(front, iter);
		if(idx == 0)
		{
			return {0u, 1u};
		}
		auto dist = std::distance(front, back);
		if(std::cmp_greater_equal(idx, dist))
		{
			return {dist - 2, dist - 1};
		}
		return {idx - 1, idx};
	}
}