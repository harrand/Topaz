#include "tz/ren/animation.hpp"

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

		// todo: add meshes and textures.

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
		// TODO: one object can only render one mesh at a time.
		// a mesh corresponds to a single gltf submesh, but a gltf node can correspond to a gltf mesh i.e multiple submeshes
		// in this case, we hit trouble because an object can only correspond to a single mesh, but we want it to correspond to multiple.
		// what we do is simple:
		// add the initial object with no mesh attached.
		// for each submesh that needs to be attached, create a new zero-TRS child object of the original, and give each of them a submesh.
		// remember that none of this is implemented yet. this is what you should do.
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

		for(std::size_t child_idx : node.children)
		{
			this->expand_current_gltf_node(gltf, child_idx, node_id);
		}
	}

	void animation_renderer::dbgui_tab_animation()
	{
		if(ImGui::BeginTabItem("Animation"))
		{
			ImGui::TextColored(ImVec4{1.0f, 0.3f, 0.3f, 1.0f}, "ANIMATION DATA");
			ImGui::Spacing();
			ImGui::Text("Coming Soon!");
			ImGui::EndTabItem();
		}
	}
}