#ifndef TZ_REN_ANIMATION_RENDERER2_HPP
#define TZ_REN_ANIMATION_RENDERER2_HPP
#include "tz/ren/mesh2.hpp"
#include "tz/io/gltf.hpp"

namespace tz::ren
{
	// an extension of mesh_renderer2.
	// this supports loading all the meshes and textures at once from a gltf.
	// once you load a gltf, you get a gltf_handle.
	// you can use a gltf_handle to create a set of objects corresponding to the gltf nodes.
	// this set of objects is called an animated object. once you create one, you get a animated_objects_handle.
	// animated_objects_handles represent a single animated object, but that may be comprised of many static subobjects (mesh_renderer2 objects) in a hierarchy.
	// some of these subobjects have meshes attached which are ultimately drawn, but many of them wont.
	// you can spawn multiple animated objects from the same gltf. they will share the same meshes and textures, but their objects will be unique.
	// this means you can have separate copies of the same animated model undergoing different points of the animations at once.
	class animation_renderer2 : private mesh_renderer2
	{
	public:
		// represents a single object (this are the subobjects that comprises animated_objects)
		using mesh_renderer2::object_handle;
		// represents a single gltf.
		using gltf_handle = tz::handle<tz::io::gltf>;
		// represents a single instance of an animated model, based off a gltf.
		using animated_objects_handle = tz::handle<object_handle>;
		struct info
		{
			std::string_view custom_vertex_spirv = {};
			std::string_view custom_fragment_spirv = {};
			tz::gl::renderer_options custom_options = {};
			std::size_t texture_capacity = 1024u;
		};
		animation_renderer2(info i);
		// update positions of all objects and animations.
		void update(float delta);
		// add a new gltf. try not to add duplicates - share gltfs as much as possible.
		// this is an expensive operation, especially if the gltf is complicated - you probably want to add these ahead-of-time.
		gltf_handle add_gltf(tz::io::gltf gltf);
		// remove a gltf.
		// please note: any animated objects that use this gltf will also be removed, meaning those animated_object_handles become invalidated.
		void remove_gltf(gltf_handle handle);

		struct animated_objects_create_info
		{
			gltf_handle gltf = tz::nullhand;
			object_handle parent = tz::nullhand;
			tz::trs local_transform = {};
		};
		animated_objects_handle add_animated_objects(animated_objects_create_info info);
	private:
		// query as to whether a gltf handle has been removed before and is still in the free list.
		bool gltf_is_in_free_list(gltf_handle handle) const;
		// advance all animated objects. invoked once per update. may run on multiple threads.
		void animation_advance(float delta);

		// represents a single animated object.
		struct animated_object_data
		{
			// gltf nodes map to objects almost 1:many.
			// nearly always its a direct 1:1 mapping, but if the node has a mesh comprised of multiple submeshes, then it maps to a single object with children with each submesh.
			std::map<std::size_t, object_handle> node_object_map = {};
			// which gltf is this animated object associated with?
			gltf_handle gltf = tz::nullhand;
			// list of all subobjects comprising this animated object.
			std::vector<object_handle> objects = {};
			// list of all animations queued up to run.
			struct animation
			{
				// which animation is currently playing?
				std::size_t animation_id = 0;
				// will this animation loop until cancelled?
				bool loop = false;
				// how fast does this animation play? 1.0 means normal speed.
				float time_warp = 1.0f;
			};
			std::vector<animation> playback = {};
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

		void gltf_load_skins(gltf_data& gltf);
		// populate all the topaz meshes (and some metadata) contained within the gltf.
		void gltf_load_meshes(gltf_data& gltf);
		// populate all the topaz textures (and some metadata) contained within the gltf.
		void gltf_load_textures(gltf_data& gltf);
		void animated_object_expand_gltf_node(animated_object_data& animated_objects, tz::io::gltf_node node, std::optional<std::size_t> parent_node_id);

		// list of all added gltfs
		std::vector<gltf_data> gltfs = {};
		// free-list for gltfs, to re-use gltf_handles.
		std::deque<gltf_handle> gltf_free_list = {};
		// list of all added animated_objects.
		std::vector<animated_object_data> animated_objects = {};
		// free-list for animated objects, to re-use animated_objects_handle.
		std::deque<animated_objects_handle> animated_objects_free_list = {};
	};
}

#endif // TZ_REN_ANIMATION_RENDERER2_HPP