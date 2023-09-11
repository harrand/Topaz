#ifndef TZ_REN_ANIMATION_HPP
#define TZ_REN_ANIMATION_HPP
#include "tz/ren/mesh.hpp"
#include "tz/io/gltf.hpp"
#include <queue>

namespace tz::ren
{
	class animation_renderer : private mesh_renderer
	{
	public:
		using gltf_handle = tz::handle<tz::io::gltf>;
		using mesh_handle = mesh_renderer::mesh_handle;
		using texture_handle = mesh_renderer::texture_handle;
		using object_handle = mesh_renderer::object_handle;
		using object_out_data = mesh_renderer::object_out_data;
		struct asset_package
		{
			gltf_handle gltfh = tz::nullhand;
			std::vector<mesh_handle> meshes = {};
			std::vector<texture_handle> textures = {};
			std::vector<object_handle> objects = {};
		};

		enum class override_flag
		{
			mesh,
			texture
		};
		using override_flags = tz::enum_field<override_flag>;

		struct override_package
		{
			asset_package pkg = asset_package{};
			override_flags overrides = {};
		};

		animation_renderer(unsigned int total_textures = 128);

		using mesh_renderer::append_to_render_graph;
		using mesh_renderer::update;
		using mesh_renderer::get_camera_transform;
		using mesh_renderer::set_camera_transform;
		using mesh_renderer::get_object;

		virtual void dbgui() override;
		void update(float delta);
		virtual object_handle add_object(object_init_data init) override;
		std::string_view get_object_name(object_handle h) const;
		std::vector<object_handle> find_objects_by_name(const char* name) const;
		tz::trs get_object_base_transform(object_handle h) const;
		void set_object_base_transform(object_handle h, tz::trs local_transform);

		asset_package add_gltf(tz::io::gltf gltf);
		asset_package add_gltf(tz::io::gltf gltf, override_package opkg);
		asset_package add_gltf(tz::io::gltf gltf, object_handle parent);
		asset_package add_gltf(tz::io::gltf gltf, object_handle parent, override_package opkg);

		std::size_t get_animation_count(const asset_package& pkg) const;
		std::optional<std::size_t> get_playing_animation(const asset_package& pkg) const;
		std::string_view get_animation_name(const asset_package& pkg, std::size_t animation_id) const;
		void play_animation(const asset_package& pkg, std::size_t animation_id, bool loop = false);
		void queue_animation(const asset_package& pkg, std::size_t animation_id, bool loop = false);
		void skip_animation(const asset_package& pkg);
		void halt_animation(const asset_package& pkg);
		float get_animation_speed(const asset_package& pkg) const;
		void set_animation_speed(const asset_package& pkg, float speed);
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
			struct metadata_t
			{
				bool has_skins = false;
				std::vector<std::size_t> mesh_submesh_indices = {};
				std::vector<std::optional<tz::io::gltf_material>> submesh_materials = {};
				std::map<std::size_t, std::size_t> joint_node_map = {};
			} metadata = {};

			struct animation_playback_t
			{
				struct queued_anim
				{
					std::size_t id;
					bool loop = false;
				};
				std::optional<std::size_t> playing_animation_id = std::nullopt;
				bool loop = false;
				std::queue<queued_anim> queued_animations = {};
				float time = 0.0f;
				float time_warp = 1.0f;
			} playback = {};

			using keyframe_iterator = std::set<tz::io::gltf_animation::keyframe_data_element>::iterator;
			std::pair<std::size_t, std::size_t> interpolate_animation_keyframes(keyframe_iterator begin, keyframe_iterator end) const;
		};

		struct object_extra_info
		{
			tz::trs base_transform = {};
			tz::trs animation_trs_offset = {};
			std::string name = "Unnamed Object";
			std::size_t submesh_count = 0;
			bool is_animated = false;
		};

		virtual void update() override;
		void expand_current_gltf_node(gltf_info& info, std::size_t node_id, std::optional<std::size_t> parent_node_id = std::nullopt, object_handle parent_override = tz::nullhand);
		void node_handle_skins(gltf_info& gltf_info);
		tz::vec2ui32 write_skin_object_data(gltf_info& gltf_info);
		void write_inverse_bind_matrices(gltf_info& gltf_info);
		void resource_write_joint_indices(gltf_info& gltf_info);
		void animation_advance(float delta);
		void shallow_patch_meshes(gltf_info& gltf_info);
		std::vector<mesh_handle> node_handle_meshes(gltf_info& gltf_info);
		std::vector<texture_handle> node_handle_materials(gltf_info& gltf_info);
		void dbgui_tab_animation();

		std::vector<gltf_info> gltfs = {};
		std::vector<object_extra_info> object_extras = {};
	};
}

#endif // TZ_REN_ANIMATION_HPP