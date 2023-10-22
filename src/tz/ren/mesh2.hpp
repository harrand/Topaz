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
			vertex_wrangler(tz::gl::renderer_info& rinfo);
			using mesh_handle = tz::handle<mesh_locator>;

			// get the vertex buffer resource
			const tz::gl::iresource& get_vertex_buffer(tz::gl::renderer_handle rh) const;
			// get the vertex buffer resource
			tz::gl::iresource& get_vertex_buffer(tz::gl::renderer_handle rh);
			// get the index buffer resource
			const tz::gl::iresource& get_index_buffer(tz::gl::renderer_handle rh) const;
			// get the index buffer resource
			tz::gl::iresource& get_index_buffer(tz::gl::renderer_handle rh);

			// get how many vertices the vertex buffer can fit in theory.
			std::size_t get_vertex_capacity(tz::gl::renderer_handle rh) const;
			// get how many indices the index buffer can fit in theory.
			std::size_t get_index_capacity(tz::gl::renderer_handle rh) const;

			// add a mesh. its vertices and indices are added into the buffers, increasing capacity if needed.
			// returns a handle signifying the mesh.
			// worst case - this can be very slow, performing upto 2 renderer edits.
			// best case - this is somewhat slow. it must perform a resource write renderer edit.
			// you should *not* do this very often.
			mesh_handle add_mesh(tz::gl::renderer_handle rh, mesh m);
			// removes a mesh, freeing up its indices/vertex to be used by someone else.
			// this is always very fast - no data is actually erased, just bookkeeping.
			void remove_mesh(mesh_handle m);
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
		};

		// this is a component of a mesh_renderer. deals with textures only.
		// provide a renderer_info to provide a set of empty image slots to your renderer.
		// once you create the renderer from that info, you can then start assigning them to images.
		class texture_manager
		{
		public:
			texture_manager(tz::gl::renderer_info& rinfo, std::size_t texture_capacity, tz::gl::resource_flags image_flags = {tz::gl::resource_flag::image_wrap_repeat});
			using texture_handle = tz::handle<tz::io::image>;

			texture_handle add_texture(tz::gl::renderer_handle rh, const tz::io::image& img);
			void assign_texture(tz::gl::renderer_handle rh, texture_handle h, const tz::io::image& img);
		private:
			texture_handle add_texture_impl(tz::gl::renderer_handle rh, tz::vec2ui dimensions, std::span<const std::byte> imgdata);
			void assign_texture_impl(tz::gl::renderer_handle rh, texture_handle th, tz::vec2ui dimensions, std::span<const std::byte> imgdata);

			std::vector<tz::gl::resource_handle> images = {};
			std::size_t texture_cursor = 0;
		};
	}

	class mesh_renderer2
	{
	public:
	private:
	};
}

#endif // TZ_REN_MESH_RENDERER2_HPP