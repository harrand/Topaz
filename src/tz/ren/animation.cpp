#include "tz/ren/animation.hpp"
#include "tz/core/job/job.hpp"

namespace tz::ren
{
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

	void animation_renderer::update()
	{
		for(std::size_t i = 0; i < mesh_renderer::draw_count(); i++)
		{
			auto maybe_tree_id = mesh_renderer::object_tree.find_node(i);
			tz::assert(maybe_tree_id.has_value());
			auto& tree_node = mesh_renderer::object_tree.get_node(maybe_tree_id.value());
			const auto& extra = this->object_extras[i];
			tree_node.local_transform = extra.base_transform.combined(extra.animation_trs_offset);
		}
		mesh_renderer::update();
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

	animation_renderer::asset_package animation_renderer::add_gltf(tz::io::gltf gltf)
	{
		// maintain offsets so we can support multiple gltfs.
		// add the new gltf.
		this->gltfs.push_back
		({
			.data = gltf,
			.object_offset = static_cast<unsigned int>(mesh_renderer::draw_count()),
		});
		auto& this_gltf = this->gltfs.back();

		// meshes
		auto meshes = this->node_handle_meshes(this_gltf);
		for(mesh_handle mesh : meshes)
		{
			this_gltf.assets.meshes.push_back(mesh);
		}
		// textures
		auto materials = this->node_handle_materials(this_gltf);
		for(texture_handle texture : materials)
		{
			this_gltf.assets.textures.push_back(texture);
		}

		// objects
		// expand nodes recursively.
		auto gltf_nodes = gltf.get_active_nodes();
		for(tz::io::gltf_node node : gltf_nodes)
		{
			// we deal with node indices, so yeah... we gotta recalculate them (boo!)
			auto iter = std::find(gltf_nodes.begin(), gltf_nodes.end(), node);
			tz::assert(iter != gltf_nodes.end());
			std::size_t node_id = std::distance(gltf_nodes.begin(), iter);

			this->expand_current_gltf_node(this_gltf, node_id);
		}

		return this_gltf.assets;
	}

	void animation_renderer::expand_current_gltf_node(gltf_info& gltf, std::size_t node_id, std::optional<std::size_t> parent_node_id)
	{
		const tz::io::gltf_node& node = gltf.data.get_nodes()[node_id];
		object_handle this_object = tz::nullhand;
		if(node.skin != static_cast<std::size_t>(-1))
		{
			// todo: skin processing.
		}
		if(parent_node_id.has_value())
		{
			// how do we figure out which object? we store it.
			this_object = this->add_object
			({
				.trs = node.transform,
				.mesh = {},
				.bound_textures = {},
				.parent = gltf.node_object_map.at(parent_node_id.value())
			});
		}
		else
		{
			// just add the object as a root node.
			this_object = this->add_object
			({
				.trs = node.transform,
				.mesh = {},
				.bound_textures = {},
			});
		}
		// new object belongs to this asset package.
		gltf.assets.objects.push_back(this_object);
		// node id also maps to this object.
		gltf.node_object_map[node_id] = this_object;

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
			}
		}

		for(std::size_t child_idx : node.children)
		{
			this->expand_current_gltf_node(gltf, child_idx, node_id);
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
					for(std::size_t i = 0; i < weight_count; i++)
					{
						ret.joint_indices[i] = vtx.jointn[i]; // + tz::vec4us::filled(this->get_gltf_node_offset());
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
			ImGui::Text("Coming Soon!");

			static int object_id = 0;

			if(mesh_renderer::draw_count() > 0)
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
					ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, objname.c_str());
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
				}
				ImGui::EndChild();
			}
			ImGui::EndTabItem();
		}
	}
}