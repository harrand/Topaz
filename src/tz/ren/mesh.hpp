#ifndef TZ_REN_mesh_renderer_HPP
#define TZ_REN_mesh_renderer_HPP
#include "tz/core/data/transform_hierarchy.hpp"
#include "tz/gl/resource.hpp"
#include "tz/io/image.hpp"
#include <deque>

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
			float texture_scale = 1.0f;
			float pad0[3];

			bool operator==(const texture_locator& rhs) const = default;
		};

		struct object_data
		{
			constexpr static std::size_t max_bound_textures = 8u;
			tz::mat4 global_transform = tz::mat4::identity();
			tz::mat4 unused;
			tz::vec3 colour_tint = tz::vec3::filled(1.0f);
			float pad0 = 0.0f;
			std::array<texture_locator, max_bound_textures> bound_textures = {};
			tz::vec4ui32 unused2 = {};
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
			using texture_locator = impl::texture_locator;
			/// Creation details for a mesh renderer's render-pass.
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

			struct object_create_info
			{
				/// Transform of the object, before parent transformations.
				tz::trs local_transform = {};
				/// Which mesh will be used? If nullhand, then this object is not rendered but still exists in the transform hierarchy.
				mesh_handle mesh = tz::nullhand;
				/// Set whether the object is initially visible or not.
				bool is_visible = true;
				/// Does this object have a parent? Nullhand if not. Global transform will be computed with respect to this parent object.
				object_handle parent = tz::nullhand;	
				/// List of all bound textures. Note that you can specify as many as you like, but any locators beyond `object_data::max_bound_textures` will be ignored.
				std::vector<texture_locator> bound_textures = {};
				/// Colour tint of this object. Does not affect children.
				tz::vec3 colour_tint = tz::vec3::filled(1.0f);
			};
			render_pass(info i);

			/// Add the underlying renderers to the end of the render graph. They will not be reliant on any other renderers.
			void append_to_render_graph();
			/// Update the transform hierarchy.
			void update();
			void dbgui_mesh();
			void dbgui_texture();
			void dbgui_objects();

			/**
			 * Retrieve the number of meshes that have been added so far.
			 * @param include_free_list Whether this number should include meshes that have been removed and are still in the free-list (false by default).
			 * @return Number of meshes stored within the mesh renderer.
			 */
			std::size_t get_mesh_count(bool include_free_list = false) const;
			/**
			 * Add a new mesh, returning the corresponding handle.
			 * @param m Mesh data comprising the mesh.
			 * @return A mesh handle corresponding to the newly-added mesh. You can then create an object using this mesh to draw it.
			 * @note This performs 2 resource writes in the best case (slow), but if capacities are exhausted and need to be resized, the worst case is an additional 2 buffer resizes (very, very slow). For this reason, you should pre-add all meshes you expect to use often, if memory allows.
			 */
			mesh_handle add_mesh(mesh m);
			/**
			 * Retrieve the mesh locator corresponding to an existing mesh.
			 * The mesh locator contains information about how much data comprises the mesh, and where in the vertex/index buffer the data resides.
			 * @param m Mesh handle to retrieve information about.
			 * @return Mesh locator corresponding to mesh_handle `m`.
			 */
			const mesh_locator& get_mesh(mesh_handle m) const;
			/**
			 * Remove a mesh from the renderer.
			 *
			 * Any objects that are currently using this mesh handle will continue to exist, but instead use an empty mesh locator, meaning it is no longer drawn.
			 * You can either remove these objects, set them to use a new mesh, or leave them in the hierarchy as they are.
			 *
			 * @param m Mesh handle to remove.
			 */
			void remove_mesh(mesh_handle m);

			/**
			 * Add a new texture to the renderer.
			 * @param img Image to add, must be in RGBA32 format.
			 * @return Handle corresponding to the newly added image.
			 * @note This performs a image-resize + resource write, which is always very slow.
			 */
			texture_handle add_texture(const tz::io::image& img);

			/**
			 * Retrieve the number of objects that have been added so far.
			 * @param include_free_list Whether this number should include objects that have been removed and are still in the free-list (false by default).
			 * @return Number of objects stored within the mesh renderer.
			 */
			std::size_t get_object_count(bool include_free_list = false) const;
			/**
			 * Add a new object.
			 * @param create Information about the object (e.g the mesh it uses, which textures it uses, what is its transform).
			 * @return Handle corresponding to the newly-added object.
			 */
			object_handle add_object(object_create_info create);
			/**
			 * Retrieve the internal shader data for an object.
			 * @param oh Handle corresponding to the object to retrieve information about.
			 * @return Reference to the shader data. You can assume any changes you make are resident in the next device draw.
			 */
			const object_data& get_object(object_handle oh) const;
			/**
			 * Retrieve the internal shader data for an object.
			 * @param oh Handle corresponding to the object to retrieve information about.
			 * @return Reference to the shader data. You can assume any changes you make are resident in the next device draw.
			 */
			object_data& get_object(object_handle oh);
			/**
			 * Remove an object.
			 * @param oh Handle corresponding to the object you want removed.
			 * @note Any child objects are also removed. Meshes/textures are not removed, even if this was the last object using them.
			 */
			void remove_object(object_handle oh);

			/**
			 * Get the local transform of a given object.
			 * @param oh Handle corresponding to the desired object.
			 * @return Transform, represented as a translation, rotation and scale.
			 * @pre There must be a valid object corresponding to the handle. If the object has been removed, or a object never existed with this handle, the behaviour is undefined.
			 */
			tz::trs object_get_local_transform(object_handle oh) const;
			/**
			 * Set the local transform of a given object.
			 * @param oh Handle corresponding to the desired object.
			 * @param trs TRS representing the new local transform of the object.
			 * @pre There must be a valid object corresponding to the handle. If the object has been removed, or a object never existed with this handle, the behaviour is undefined.
			 */
			void object_set_local_transform(object_handle oh, tz::trs trs);
			/**
			 * Get the global transform of a given object. Think of this as the world-space transform.
			 * @param oh Handle corresponding to the desired object.
			 * @return Transform, represented as a translation, rotation and scale.
			 * @pre There must be a valid object corresponding to the handle. If the object has been removed, or a object never existed with this handle, the behaviour is undefined.
			 */
			tz::trs object_get_global_transform(object_handle oh) const;
			/**
			 * Set the global transform of a given object.
			 * @param oh Handle corresponding to the desired object.
			 * @param trs TRS representing the new global transform of the object.
			 * @pre There must be a valid object corresponding to the handle. If the object has been removed, or a object never existed with this handle, the behaviour is undefined.
			 * @note It is much faster to set the local transform than it is to set the global transform with this method. Prefer @ref object_set_local_transform unless you must work in world-space.
			 */
			void object_set_global_transform(object_handle oh, tz::trs trs);

			texture_locator object_get_texture(object_handle oh, std::size_t bound_texture_id) const;
			void object_set_texture(object_handle oh, std::size_t bound_texture_id, texture_locator tloc);
			bool object_get_visible(object_handle oh) const;
			void object_set_visible(object_handle oh, bool visible);

			/**
			 * Retrieve the underlying transform hierarchy.
			 * 
			 * Implementation details are as follows:
			 * - The hierarchy represents the objects as nodes in a tree. Each node contains a local transform, and a data payload.
			 * - The data payload corresponds to the integral value of the object's corresponding object-handle (i.e `static_cast<std::size_t>(static_cast<tz::hanval>(my_object_handle))`).
			 * - If an object is removed, its corresponding node is deleted.
			 */
			const tz::transform_hierarchy<std::uint32_t>& get_hierarchy() const;
			/**
			 * Retrieve the underlying transform hierarchy.
			 * 
			 * Implementation details are as follows:
			 * - The hierarchy represents the objects as nodes in a tree. Each node contains a local transform, and a data payload.
			 * - The data payload corresponds to the integral value of the object's corresponding object-handle (i.e `static_cast<std::size_t>(static_cast<tz::hanval>(my_object_handle))`).
			 * - If an object is removed, its corresponding node is deleted.
			 */
			tz::transform_hierarchy<std::uint32_t>& get_hierarchy();

			/**
			 * Retrieve the resource handle corresponding to an extra buffer that was passed in @ref info during creation of the mesh renderer.
			 * @param extra_buffer_id Id of the extra buffer to retrieve. Note, this id associates the buffer with the expression `info::extra_buffers[id]` at the point of mesh renderer creation.
			 * @return Handle corresponding to the `extra_buffer_id`'th index of the extra buffers provided at mesh renderer creation time.
			 * @pre `extra_buffer_id < get_extra_buffer_count()`, or the behaviour is undefined.
			 */
			tz::gl::resource_handle get_extra_buffer(std::size_t extra_buffer_id) const;
			/**
			 * Retrieve the number of extra buffers that were specified when the mesh renderer was created.
			 * @return Number of extra buffers.
			 */
			std::size_t get_extra_buffer_count() const;

			tz::trs get_camera_transform() const;
			void set_camera_transform(tz::trs camera_transform);

			struct camera_perspective_t
			{
				float aspect_ratio = 1.0f;
				float fov = 1.5701f;
				float near_clip = 0.1f;
				float far_clip = 100.0f;
			};
			struct camera_orthographic_t
			{
				float left, right;
				float top, bottom;
				float near_plane, far_plane;	
			};

			void camera_perspective(camera_perspective_t persp);
			void camera_orthographic(camera_orthographic_t ortho);

			/// Retrieve the renderer handle associated with the compute pre-pass. This pass populates the indirect draw list.
			tz::gl::renderer_handle get_compute_pass() const;
			/// Retrieve the renderer handle associated with the mesh renderer's main render pass.
			tz::gl::renderer_handle get_render_pass() const;
		protected:
			bool object_is_in_free_list(object_handle oh) const;
		private:
			compute_pass compute;
			tz::gl::renderer_handle render = tz::nullhand;
			tz::gl::resource_handle camera_buffer = tz::nullhand;
			tz::gl::resource_handle draw_indirect_ref = tz::nullhand;
			vertex_wrangler vtx = {};
			texture_manager tex = {};
			object_storage obj = {};
			tz::transform_hierarchy<std::uint32_t> tree = {};
			std::optional<std::size_t> extra_buf_hanval_first = std::nullopt;
			std::optional<std::size_t> extra_buf_hanval_last = std::nullopt;
			int dbgui_object_cursor = 0;
		};
	}

	/**
	* @ingroup tz_ren
	* A lightweight 3D mesh renderer. If you'd like to render animated models, you're looking for @ref animation_renderer.
	*
	* Mesh Renderers are comprised of three major components:
	* <Mesh>
	* - Represents a triangulated set of indices and vertices. These are necessary to give shape to drawn geometry.
	* - First you must fill in the data of a `mesh_renderer::mesh`, and then add it using `mesh_renderer::add_mesh`. Keep ahold of the returned `mesh_handle`.
	* 
	* <Texture>
	* - Represents an image that is sampled when drawing a mesh.
	* - First you must fill the data of a `tz::io::image`, in the format `tz::gl::image_format::RGBA32`, and then add it using `mesh_renderer::add_texture`. Keep ahold of the returned `texture_handle`.
	*
	* <Object>
	* - Represents a "thing" that is drawn in the 3D world.
	* - Objects can have zero or more child objects. That is - objects are expressed as a graph, you can have as many root nodes as you like, and each object can have as many children as you like.
	* - At its core, objects are comprised of:
	* 	- A local transform (its position, rotation and scale with respect to its parent)
	* 	- (Optional) A mesh (when you draw the object, what geometry should be drawn as its location?).
	*		- If you don't want the object to be drawn, and just to exist as a node within the hierarchy, you can set the mesh to null.
	*	- A list of bound textures. These textures will be sampled from when drawing the object's associated mesh.
	*		- The list should have a size less-than-or-equal to the value `tz::ren::impl::object_data::max_bound_textures`. Any excess texture locators will be ignored.
	* - To add a new object to the 3D world, invoke `mesh_renderer::add_object`. Keep ahold of the returned `object_handle`.
	*/
	class mesh_renderer : public impl::render_pass
	{
	public:
		using info = impl::render_pass::info;
		using mesh = impl::mesh;
		using mesh_locator = impl::mesh_locator;
		/**
		 * Create a new mesh renderer.
		 **/
		mesh_renderer(info i = {});
		void dbgui(bool include_operations = true);
		void dbgui_operations();
	};
}

#endif // TZ_REN_mesh_renderer_HPP