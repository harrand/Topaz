#ifndef TZ_REN_MESH_RENDERER2_HPP
#define TZ_REN_MESH_RENDERER2_HPP
#include "tz/core/data/transform_hierarchy.hpp"
#include "tz/io/image.hpp"

namespace tz::ren
{
	namespace impl
	{
		// represents a single vertex of a mesh_renderer
		struct mesh_vertex
		{
			tz::vec3 position;
			float pad0;
			tz::vec2 texcoord;
			float pad1[2];
			tz::vec3 normal;
			float pad2;
			tz::vec3 tangent;
			float pad3;
			tz::vec4ui32 joint_indices;
			tz::vec4 joint_weights;
		};

		// represents a single index of a mesh_renderer
		using mesh_index = std::uint32_t;

		// a mesh for a mesh_renderer
		struct mesh
		{
			std::vector<mesh_vertex> vertices = {};
			std::vector<mesh_index> indices = {};
		};

		// describes the location and dimensions of a mesh within the giant vertex and index buffers.
		struct mesh_locator
		{
			// how far into the vertex buffer does this mesh's vertices live?
			std::uint32_t vertex_offset = 0;
			// how many vertices does this mesh have?
			std::uint32_t vertex_count = 0;
			// how far into the index buffer does this mesh's indices live?
			std::uint32_t index_offset = 0;	
			// how many indices does this mesh have?
			std::uint32_t index_count = 0;
			// X, where all indices of this mesh are between 0 and X.
			std::uint32_t max_index_value = 0;
			bool operator==(const mesh_locator& rhs) const = default;
		};

		// this is a component of a mesh_renderer. deals entirely with the vertex/index logic, and nothing else.
		// provide a renderer_info to attach a vertex/index buffer to your renderer.
		// once you create the renderer from that info, you can call these methods with it to request info/do operations.
		class vertex_wrangler
		{
		public:
			vertex_wrangler() = default;
			vertex_wrangler(tz::gl::renderer_info& rinfo);
			using mesh_handle = tz::handle<mesh_locator>;

			// get how many vertices the vertex buffer can fit in theory.
			std::size_t get_vertex_capacity(tz::gl::renderer_handle rh) const;
			// get how many indices the index buffer can fit in theory.
			std::size_t get_index_capacity(tz::gl::renderer_handle rh) const;
			// calculate how many vertices are actively being used by added meshes.
			std::size_t get_vertex_count() const;
			// calculate how many indices are actively being used by added meshes.
			std::size_t get_index_count() const;

			// add a mesh. its vertices and indices are added into the buffers, increasing capacity if needed.
			// returns a handle signifying the mesh.
			// worst case - this can be very slow, performing upto 2 renderer edits.
			// best case - this is somewhat slow. it must perform a resource write renderer edit.
			// you should *not* do this very often.
			mesh_handle add_mesh(tz::gl::renderer_handle rh, mesh m);
			const mesh_locator& get_mesh(mesh_handle h) const;
			mesh_handle try_find_mesh_handle(const mesh_locator& loc) const;
			std::size_t get_mesh_count(bool include_free_list = false) const;
			// removes a mesh, freeing up its indices/vertex to be used by someone else.
			// this is always very fast - no data is actually erased, just bookkeeping.
			void remove_mesh(mesh_handle m);
			void dbgui(tz::gl::renderer_handle rh);
		private:
			// return element-offset into vertex buffer that could fit the required vertices, or nullopt if there isn't enough space.
			std::optional<std::uint32_t> try_find_vertex_region(tz::gl::renderer_handle rh, std::size_t vertex_count) const;
			// return element-offset into index buffer that could fit the required indices, or nullopt if there isn't enough space.
			std::optional<std::uint32_t> try_find_index_region(tz::gl::renderer_handle rh, std::size_t index_count) const;
			mesh_locator add_mesh_impl(tz::gl::renderer_handle rh, const mesh& m);

			static constexpr std::size_t initial_vertex_capacity = 1024u;
			tz::gl::resource_handle vertex_buffer = tz::nullhand;
			tz::gl::resource_handle index_buffer = tz::nullhand;

			std::vector<mesh_locator> mesh_locators = {};
			std::deque<tz::hanval> mesh_handle_free_list = {};
			std::size_t added_vertex_count = 0;
			std::size_t added_index_count = 0;
			int dbgui_mesh_cursor = 0;
		};

		// this is a component of a mesh_renderer. deals with textures only.
		// provide a renderer_info to provide a set of empty image slots to your renderer.
		// once you create the renderer from that info, you can then start assigning them to images.
		class texture_manager
		{
		public:
			texture_manager() = default;
			texture_manager(tz::gl::renderer_info& rinfo, std::size_t texture_capacity, tz::gl::resource_flags image_flags = {tz::gl::resource_flag::image_wrap_repeat});
			using texture_handle = tz::handle<tz::io::image>;

			texture_handle add_texture(tz::gl::renderer_handle rh, const tz::io::image& img);
			void assign_texture(tz::gl::renderer_handle rh, texture_handle h, const tz::io::image& img);
			void dbgui(tz::gl::renderer_handle rh);
			std::size_t get_texture_count() const;
			std::size_t get_texture_capacity() const;
		private:
			texture_handle add_texture_impl(tz::gl::renderer_handle rh, tz::vec2ui dimensions, std::span<const std::byte> imgdata);
			void assign_texture_impl(tz::gl::renderer_handle rh, texture_handle th, tz::vec2ui dimensions, std::span<const std::byte> imgdata);

			std::vector<tz::gl::resource_handle> images = {};
			std::size_t texture_cursor = 0;
		};

		// this is a component of a mesh_renderer. deals with a compute pre-pass to generate draw commands.
		// this has its own compute pass stored internally. you're gonna want to include this as a member in the mesh renderer and then use its API accordingly.
		// theres alot of methods with overlapping interests.
		// generally, when you wanna add N new objects:
		// just use add_new_draws(N), and
		// then for i=0,N,++ set_mesh_at(object_id, whichever_mesh_locator_you_want)
		// please make sure the mesh_locator maps to something the vertex_wrangler gives you... otherwise shit is bound to get corrupted.
		class compute_pass
		{
		public:
			compute_pass();
			tz::gl::renderer_handle get_compute_pass() const;
			// get the number of draws (including empty free-listed draws).
			std::size_t get_draw_count() const;
			// get the maximum number of draws without resizing.
			std::size_t get_draw_capacity() const;
			// change the draw capacity to something new.
			// this is always very slow. 2 renderer edits.
			void set_draw_capacity(std::size_t new_capacity);
			// set the mesh of an existing draw to something else. does not affect draw count. draw_id needs to be less than draw count.
			mesh_locator get_mesh_at(std::size_t draw_id) const;
			void set_mesh_at(std::size_t draw_id, mesh_locator loc);
			bool get_visibility_at(std::size_t draw_id) const;
			void set_visibility_at(std::size_t draw_id, bool visible);
			// add N new draws. increments the draw count N times. if draw capacity is too small, increase it by N or double its capacity, whichever is bigger.
			// mesh at the new draw will be an empty mesh locator. you're free to change it to something else.
			// returns the draw-id of the first new draw.
			// worst case - very slow. needs to do a renderer edit.
			// best case - very fast. just writes into a dynamic buffer resource.
			std::vector<std::size_t> add_new_draws(std::size_t number_of_new_draws);
			void remove_draw(std::size_t draw_id);

			tz::gl::resource_handle get_draw_indirect_buffer() const;
			std::size_t get_draw_free_list_count() const;
			bool is_in_free_list(std::size_t draw_id) const;
			static constexpr std::size_t initial_max_draw_count = 1024u;
		private:
			// sets the draw count to something new.
			// `new_draw_count` must be less than the current draw capacity, or this will assert.
			// also, the mesh locators of the new draws will be an indeterminate value. you should probably make sure they're empty yourself.
			// if you dont want to deal with this, use `add_new_draws` instead.
			void set_draw_count(std::size_t new_draw_count);

			tz::gl::resource_handle draw_indirect_buffer = tz::nullhand;
			tz::gl::resource_handle mesh_locator_buffer = tz::nullhand;
			tz::gl::resource_handle draw_visibility_buffer = tz::nullhand;
			tz::gl::renderer_handle compute = tz::nullhand;
			std::deque<std::size_t> draw_id_free_list = {};
		};

		// represents one of the textures bound to an object (drawable)
		struct texture_locator
		{
			bool is_null() const{return this->texture == tz::nullhand;}
			// colour multiplier on the sampled texel
			tz::vec3 colour_tint = tz::vec3::filled(1.0f);
			// id within the overarching texture resource array to be sampled.
			texture_manager::texture_handle texture = tz::nullhand;

			bool operator==(const texture_locator& rhs) const = default;
		};

		struct object_data
		{
			constexpr static std::size_t max_bound_textures = 8u;
			tz::mat4 global_transform = tz::mat4::identity();
			tz::mat4 unused;
			tz::vec3 colour_tint = tz::vec3::filled(1.0f);
			float pad0;
			std::array<texture_locator, max_bound_textures> bound_textures = {};
			tz::vec4ui32 unused2;
		};

		// objects represent a single renderable thing.
		// they are comprised of:
		// a transform (as part of a full transform hierarchy)
		//	- this is a part of the object tree.
		//	- the global transform of the object based on the hierarchy is computed every frame. cpu-side.
		// a fixed-size array of texture locators. essentially an object can use multiple textures. useful for normal-maps etc...
		//	- this also contains a tint.
		//	- texture locators are part of the object data.
		// an optional mesh!
		//	- the mesh of object X corresponds to the mesh_locator at index X of the mesh_locator_buffer of the compute_pass.
		//	- an empty (default) mesh_locator means that the object exists in the hierarchy, but isn't meant to be rendered (e.g bones in a skeletal animation)

		// this is a component of a mesh_renderer.
		// deals with object storage and how they are rendered.
		class object_storage
		{
		public:
			object_storage() = default;
			object_storage(tz::gl::renderer_info& rinfo);

			using object_handle = tz::handle<object_data>;
			// note: get_object_capacity should always be equal to compute_pass::get_draw_capacity.
			// this is because every object must have a corresponding mesh locator associated with it.
			std::size_t get_object_capacity(tz::gl::renderer_handle rh) const;
			// change the object capacity
			// do this whenever you set_draw_count on the compute pass - these need to be synchronised.
			// note: any new objects resulting from an increase of capacity will be implicitly defaulted.
			void set_object_capacity(tz::gl::renderer_handle rh, std::size_t new_capacity);
			// you can read/write to object data at any time (note: not thread safe).
			std::span<const object_data> get_object_internals(tz::gl::renderer_handle rh) const;
			std::span<object_data> get_object_internals(tz::gl::renderer_handle rh);
		private:
			tz::gl::resource_handle object_buffer = tz::nullhand;
		};

		// this is a component of a mesh_renderer. deals with the main tz::gl::renderer.
		// this should make it easier to maintain and configure.
		class render_pass
		{
		public:
			using mesh_handle = vertex_wrangler::mesh_handle;
			using texture_handle = texture_manager::texture_handle;
			using object_handle = object_storage::object_handle;
			struct info
			{
				std::string_view custom_vertex_spirv = {};
				std::string_view custom_fragment_spirv = {};
				tz::gl::renderer_options custom_options = {};
				std::size_t texture_capacity = 1024u;
			};

			struct object_create_info
			{
				tz::trs local_transform = {};
				mesh_handle mesh = tz::nullhand;
				bool is_visible = true;
				object_handle parent = tz::nullhand;	
				std::vector<texture_locator> bound_textures = {};
				tz::vec3 colour_tint = tz::vec3::filled(1.0f);
			};
			render_pass(info i);

			void append_to_render_graph();
			void update();
			void dbgui_mesh();
			void dbgui_texture();
			void dbgui_objects();

			std::size_t get_mesh_count(bool include_free_list = false) const;
			mesh_handle add_mesh(mesh m);
			const mesh_locator& get_mesh(mesh_handle m) const;
			void remove_mesh(mesh_handle m);

			texture_handle add_texture(const tz::io::image& img);

			std::size_t get_object_count(bool include_free_list = false) const;
			object_handle add_object(object_create_info create);
			const object_data& get_object(object_handle oh) const;
			object_data& get_object(object_handle oh);
			void remove_object(object_handle oh);
		protected:
			bool object_is_in_free_list(object_handle oh) const;
		private:
			compute_pass compute;
			tz::gl::renderer_handle render = tz::nullhand;
			tz::gl::resource_handle draw_indirect_ref = tz::nullhand;
			vertex_wrangler vtx = {};
			texture_manager tex = {};
			object_storage obj = {};
			tz::transform_hierarchy<std::uint32_t> tree = {};
			int dbgui_object_cursor = 0;
		};
	}

	class mesh_renderer2 : public impl::render_pass
	{
	public:
		using info = impl::render_pass::info;
		using mesh = impl::mesh;
		using mesh_locator = impl::mesh_locator;
		mesh_renderer2(info i = {});
		void dbgui(bool include_operations = true);
		void dbgui_operations();
	};
}

#endif // TZ_REN_MESH_RENDERER2_HPP