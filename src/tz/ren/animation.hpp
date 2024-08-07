#ifndef TZ_REN_animation_renderer_HPP
#define TZ_REN_animation_renderer_HPP
#include "tz/ren/mesh.hpp"
#include "tz/io/gltf.hpp"
#include "tz/core/job/job.hpp"
#include <map>

namespace tz::ren
{
	/**
	 * @ingroup tz_ren
	 * A superset of @ref mesh_renderer
	 * an extension of mesh_renderer.
	 * this supports loading all the meshes and textures at once from a gltf.
	 * once you load a gltf, you get a gltf_handle.
	 * you can use a gltf_handle to create a set of objects corresponding to the gltf nodes.
	 * this set of objects is called an animated object. once you create one, you get a animated_objects_handle.
	 * animated_objects_handles represent a single animated object, but that may be comprised of many static subobjects (mesh_renderer objects) in a hierarchy.
	 * some of these subobjects have meshes attached which are ultimately drawn, but many of them wont.
	 * you can spawn multiple animated objects from the same gltf. they will share the same meshes and textures, but their objects will be unique.
	 * this means you can have separate copies of the same animated model undergoing different points of the animations at once.
	 */
	class animation_renderer : private mesh_renderer
	{
	public:
		// represents a single object (this are the subobjects that comprises animated_objects)
		using mesh_renderer::object_handle;
		using mesh_renderer::texture_handle;
		using mesh_renderer::texture_locator;
		// represents a single gltf.
		using gltf_handle = tz::handle<tz::io::gltf>;
		// represents a single instance of an animated model, based off a gltf.
		using animated_objects_handle = tz::handle<object_handle>;
		struct info
		{
			/// String representing SPIRV for the vertex shader. If empty, a default vertex shader is used.
			std::string_view custom_vertex_spirv = {};
			/// String representing SPIRV for the fragment shader. If empty, a default fragment shader is used - displays the textured objects with no lighting effects.
			std::string_view custom_fragment_spirv = {};
			/// If you want more fine-grained control over the created graphics renderer pass (such as if you want to add a post-process effect), you can add extra options here.
			tz::gl::renderer_options custom_options = {};
			/// Maximum number of textures. Note that this capacity cannot be expanded - make sure you never exceed this limit.
			std::size_t texture_capacity = 1024u;
			/// A list of extra buffer resources. These buffers will be resident to both the vertex and fragment shader. Resource ID of the first extra buffer will be `3` ascending.
			std::vector<tz::gl::buffer_resource> extra_buffers = {};
			/// Optional output. Use this if you want to render into a specific render target. If this is nullptr, it will render directly into the window instead.
			tz::gl::ioutput* output = nullptr;
		};
		animation_renderer(info i);
		animation_renderer();
		// update positions of all objects and animations.
		void update(float delta);
		// blocks the current thread until all remaining animation advance async work has completed.
		void block();
		void dbgui();
		// add a new gltf. try not to add duplicates - share gltfs as much as possible.
		// this is an expensive operation, especially if the gltf is complicated - you probably want to add these ahead-of-time.
		gltf_handle add_gltf(tz::io::gltf gltf);
		// remove a gltf.
		// please note: any animated objects that use this gltf will also be removed, meaning those animated_object_handles become invalidated.
		void remove_gltf(gltf_handle handle);
		using mesh_renderer::append_to_render_graph;

		struct playback_data
		{
			// which animation is currently playing?
			std::size_t animation_id = 0;
			// will this animation loop until cancelled?
			bool loop = false;
			// how fast does this animation play? 1.0 means normal speed.
			float time_warp = 1.0f;
		};

		struct animated_objects_create_info
		{
			gltf_handle gltf = tz::nullhand;
			object_handle parent = tz::nullhand;
			tz::trs local_transform = {};
		};
		animated_objects_handle add_animated_objects(animated_objects_create_info info);
		void remove_animated_objects(animated_objects_handle handle);

		// API get for gltf
		std::size_t gltf_get_animation_count(gltf_handle h) const;
		std::string_view gltf_get_animation_name(gltf_handle h, std::size_t anim_id) const;
		float gltf_get_animation_length(gltf_handle h, std::size_t anim_id) const;
		std::optional<std::size_t> gltf_get_animation_id_by_name(gltf_handle h, std::string anim_name) const;

		// API get for animated_objects

		std::span<const object_handle> animated_object_get_subobjects(animated_objects_handle handle) const;
		gltf_handle animated_object_get_gltf(animated_objects_handle handle) const;
		float animated_object_get_playback_time(animated_objects_handle handle) const;
		void animated_object_set_playback_time(animated_objects_handle handle, float time);
		std::span<const playback_data> animated_object_get_playing_animations(animated_objects_handle handle) const;
		std::span<playback_data> animated_object_get_playing_animations(animated_objects_handle handle);
		void animated_object_skip_all_animations(animated_objects_handle handle);
		void animated_object_play_animation(animated_objects_handle handle, playback_data anim);
		bool animated_object_play_animation_by_name(animated_objects_handle handle, std::string_view name, playback_data anim);
		void animated_object_queue_animation(animated_objects_handle handle, playback_data anim);
		bool animated_object_queue_animation_by_name(animated_objects_handle handle, std::string_view name, playback_data anim);
		void animated_object_skip_animation(animated_objects_handle handle);

		tz::trs animated_object_get_local_transform(animated_objects_handle handle) const;
		void animated_object_set_local_transform(animated_objects_handle handle, tz::trs trs);
		tz::trs animated_object_get_global_transform(animated_objects_handle handle) const;
		void animated_object_set_global_transform(animated_objects_handle handle, tz::trs trs);

		using mesh_renderer::add_texture;
		using mesh_renderer::add_object;
		using mesh_renderer::remove_object;
		using mesh_renderer::get_object;
		using mesh_renderer::object_get_local_transform;
		using mesh_renderer::object_set_local_transform;
		using mesh_renderer::object_get_global_transform;
		using mesh_renderer::object_set_global_transform;
		using mesh_renderer::get_compute_pass;
		using mesh_renderer::get_render_pass;
		using mesh_renderer::object_get_texture;
		using mesh_renderer::object_set_texture;
		using mesh_renderer::object_get_visible;
		using mesh_renderer::object_set_visible;
		using mesh_renderer::object_get_parent;
		using mesh_renderer::object_set_parent;
		using mesh_renderer::get_camera_transform;
		using mesh_renderer::set_camera_transform;
		using mesh_renderer::camera_perspective;
		using mesh_renderer::camera_orthographic;
		using mesh_renderer::get_extra_buffer;
		using mesh_renderer::get_extra_buffer_count;
	private:
		// query as to whether a gltf handle has been removed before and is still in the free list.
		bool gltf_is_in_free_list(gltf_handle handle) const;
		bool animated_objects_are_in_free_list(animated_objects_handle handle) const;
		// advance all animated objects. invoked once per update. may run on multiple threads.
		void animation_advance(float delta);
		void single_animation_advance(float delta, animated_objects_handle h);
		tz::gl::resource_handle get_joint_buffer_handle() const;

		// represents a single animated object.
		struct animated_object_data
		{
			// gltf nodes map to objects almost 1:many.
			// nearly always its a direct 1:1 mapping, but if the node has a mesh comprised of multiple submeshes, then it maps to a single object with children with each submesh.
			std::map<std::size_t, object_handle> node_object_map = {};
			// each animated object owns a part of the joint buffer.
			// the next 2 variables comprise this owned region.
			// at `joint_buffer_offset` indices into the joint buffer, an array of object-ids can be found (of size joint_count)
			// the i'th value of this array represents the object-id corresponding to the i'th joint for this particular animated object.
			std::size_t joint_buffer_offset = 0;
			// if joint_count is zero, then this object is not really animated (this must match the joint count of the corresponding gltf skin).
			std::size_t joint_count = 0;
			// which gltf is this animated object associated with?
			gltf_handle gltf = tz::nullhand;
			// list of all subobjects comprising this animated object.
			std::vector<object_handle> objects = {};
			// list of all animations queued up to run.
			std::vector<playback_data> playback = {};
			// how many seconds have elapsed since the currently-playing animation began?
			float playback_time = 0.0f;
		};

		// represents a single gltf.
		struct gltf_data
		{
			// the uh gltf itself.
			tz::io::gltf data = {};
			// some metadata, most of these are to help add the textures/meshes properly.
			// dont fuck with these.
			struct metadata_t
			{
				std::unordered_map<std::string, std::size_t> animation_name_to_id_map = {};
				// does this gltf contain at least one skin (i.e skinned mesh)
				bool has_skins = false;
				// one entry per mesh. represents the cumulative number of submeshes processed while iterating through the gltf meshes.
				std::vector<std::size_t> mesh_submesh_indices = {};
				// one entry per submesh. contains the associated material for each submesh, or nullhand if it doesnt have one.
				std::vector<std::optional<tz::io::gltf_material>> submesh_materials = {};
				// maps joint indices to node ids. animated objects will know how to map gltf node ids to object ids
				// the shader will need to be able to map joint indices to object ids, hence this intermediate container.
				std::map<std::size_t, std::size_t> joint_node_map = {};
			} metadata;
			// list of all submeshes within the gltf. note that a gltf mesh does not map to a mesh_handle, but each of its submeshes does.
			std::vector<mesh_handle> meshes = {};
			// list of all textures existing within the gltf. no information about what they're meant to represent specifically.
			// the gltf_materials contain more information about these texture ids.
			std::vector<texture_handle> textures = {};
		};
		// in our constructor we could have extra buffers specified.
		// however, as an animation renderer, we also have our own extra buffers we need to add before the other extras.
		// this method combines them.
		static std::vector<tz::gl::buffer_resource> evaluate_extra_buffers(const info& i);
		void gltf_load_skins(gltf_data& gltf);
		// populate all the topaz meshes (and some metadata) contained within the gltf.
		void gltf_load_meshes(gltf_data& gltf);
		// populate all the topaz textures (and some metadata) contained within the gltf.
		void gltf_load_textures(gltf_data& gltf);
		void animated_object_expand_gltf_node(animated_object_data& animated_objects, tz::io::gltf_node node, std::optional<std::size_t> parent_node_id);
		void animated_object_write_inverse_bind_matrices(animated_object_data& animated_objects);
		tz::vec2ui32 animated_object_write_joints(animated_object_data& animated_objects);
		std::optional<std::size_t> try_find_joint_region(std::size_t joint_count) const;
		std::size_t get_joint_count() const;
		std::size_t get_joint_capacity() const;
		void set_joint_capacity(std::size_t new_joint_capacity);
		void wait_for_animation_jobs();
		void dbgui_animations();
		void dbgui_animation_operations();

		// list of all added gltfs
		std::vector<gltf_data> gltfs = {};
		// free-list for gltfs, to re-use gltf_handles.
		std::deque<gltf_handle> gltf_free_list = {};
		// list of all added animated_objects.
		std::vector<animated_object_data> animated_objects = {};
		// free-list for animated objects, to re-use animated_objects_handle.
		std::deque<animated_objects_handle> animated_objects_free_list = {};
		std::vector<tz::job_handle> animation_advance_jobs = {};
		int dbgui_animated_objects_cursor = 0;
	};
}

#endif // TZ_REN_animation_renderer_HPP