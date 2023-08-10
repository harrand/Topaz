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
		void append_to_render_graph();
		void dbgui();
	private:
		struct compute_pass_t
		{
			compute_pass_t();

			tz::gl::resource_handle draw_indirect_buffer = tz::nullhand;
			tz::gl::resource_handle draw_list_buffer = tz::nullhand;
			tz::gl::renderer_handle handle = tz::nullhand;
		};
		struct render_pass_t
		{
			render_pass_t(tz::gl::renderer_handle compute_pass, tz::gl::resource_handle compute_draw_indirect_buffer, unsigned int total_textures = 128);
			tz::gl::resource_handle index_buffer = tz::nullhand;
			tz::gl::resource_handle vertex_buffer = tz::nullhand;
			tz::gl::resource_handle object_buffer = tz::nullhand;
			tz::gl::resource_handle camera_buffer = tz::nullhand;
			tz::gl::resource_handle draw_indirect_buffer_ref = tz::nullhand;
			std::vector<tz::gl::resource_handle> textures = {};
			tz::gl::renderer_handle handle = tz::nullhand;
		};

		void dbgui_impl();

		compute_pass_t compute_pass = {};
		render_pass_t render_pass;
	};
}

#endif // TOPAZ_REN_MESH_HPP