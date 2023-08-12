#ifndef TOPAZ_REN_MESH_HPP
#define TOPAZ_REN_MESH_HPP
#include "tz/gl/renderer.hpp"
#include "tz/core/data/vector.hpp"
#include <cstdint>
#include <array>

namespace tz::ren
{
	template<unsigned int max_tex_count = 8>
	struct vertex
	{
		tz::vec3 position = tz::vec3::zero();
		float pad;
		std::array<tz::vec4, max_tex_count> texcoordn = {};
		tz::vec3 normal;
		float pad0;
		tz::vec3 tangent;
		float pad1;
	};

	using index = std::uint32_t;

	template<unsigned int max_tex_count = 8>
	struct mesh
	{
		std::vector<vertex<max_tex_count>> vertices = {};
		std::vector<index> indices = {};
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

	/**
	* @ingroup tz_ren
	* todo: document
	* a two-pass (compute gpu command generation => render) mesh renderer. pbr?
	*/
	constexpr unsigned int mesh_renderer_max_tex_count = 8;
	class mesh_renderer
	{
	public:
		mesh_renderer(unsigned int total_textures = 128);
		using vertex_t = vertex<mesh_renderer_max_tex_count>;
		using mesh_t = mesh<mesh_renderer_max_tex_count>;
		using mesh_handle_t = tz::handle<mesh_t>;

		std::size_t mesh_count() const;
		std::size_t object_count() const;
		mesh_handle_t add_mesh(mesh_t m);
		void append_to_render_graph();
		void dbgui();
	private:
		struct compute_pass_t
		{
			compute_pass_t();
			std::span<const mesh_locator> get_draw_list_meshes() const;
			std::span<mesh_locator> get_draw_list_meshes();
			std::uint32_t get_draw_count() const;
			void set_draw_count(std::uint32_t new_draw_count);
			void dbgui();

			tz::gl::resource_handle draw_indirect_buffer = tz::nullhand;
			tz::gl::resource_handle draw_list_buffer = tz::nullhand;
			tz::gl::renderer_handle handle = tz::nullhand;
		};
		struct render_pass_t
		{
			render_pass_t(tz::gl::renderer_handle compute_pass, tz::gl::resource_handle compute_draw_indirect_buffer, unsigned int total_textures = 128);
			void dbgui();
			tz::gl::iresource* get_index_buffer_resource();
			tz::gl::iresource* get_vertex_buffer_resource();

			tz::gl::resource_handle index_buffer = tz::nullhand;
			tz::gl::resource_handle vertex_buffer = tz::nullhand;
			tz::gl::resource_handle object_buffer = tz::nullhand;
			tz::gl::resource_handle camera_buffer = tz::nullhand;
			tz::gl::resource_handle draw_indirect_buffer_ref = tz::nullhand;
			std::vector<tz::gl::resource_handle> textures = {};
			tz::gl::renderer_handle handle = tz::nullhand;
		};

		std::optional<std::uint32_t> try_find_index_section(std::size_t index_count) const;
		std::optional<std::uint32_t> try_find_vertex_section(std::size_t vertex_count) const;
		mesh_locator add_mesh_impl(const mesh_renderer::mesh_t& m);
		void dbgui_impl();

		compute_pass_t compute_pass = {};
		render_pass_t render_pass;
		// not to be confused with the draw-list (part of the compute pass), which only contains the mesh locators that are gonna be drawn. this contains every mesh ever added.
		std::vector<mesh_locator> meshes = {};
	};
}

#endif // TOPAZ_REN_MESH_HPP