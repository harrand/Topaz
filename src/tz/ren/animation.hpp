#ifndef TZ_REN_ANIMATION_HPP
#define TZ_REN_ANIMATION_HPP
#include "tz/ren/mesh.hpp"
#include "tz/io/gltf.hpp"

namespace tz::ren
{
	class animation_renderer : private mesh_renderer
	{
	public:
		using gltf_handle = tz::handle<tz::io::gltf>;
		struct asset_package
		{
			gltf_handle gltf = tz::nullhand;
			std::vector<mesh_handle> meshes = {};
			std::vector<texture_handle> textures = {};
			std::vector<object_handle> objects = {};
		};

		animation_renderer() = default;

		using mesh_renderer::append_to_render_graph;
		using mesh_renderer::update;
		using mesh_renderer::dbgui;

		asset_package add_gltf(tz::io::gltf gltf);
	private:
		struct gltf_info
		{
			tz::io::gltf data;
			// represents the number of objects that exist before the first object corresponding to this gltf.
			unsigned int object_offset;
			// maps each node id of this gltf to an object handle within the scene.
			std::map<std::size_t, object_handle> node_object_map = {};
			// stores each asset that was brought in by this gltf.
			asset_package assets = {};
		};
		void expand_current_gltf_node(gltf_info& info, std::size_t node_id, std::optional<std::size_t> parent_node_id = std::nullopt);
		std::vector<gltf_info> gltfs = {};
	};
}

#endif // TZ_REN_ANIMATION_HPP