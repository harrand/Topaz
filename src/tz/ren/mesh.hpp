#ifndef TOPAZ_REN_MESH_HPP
#define TOPAZ_REN_MESH_HPP
#include "tz/gl/renderer.hpp"
#include "tz/core/data/vector.hpp"
#include "tz/core/matrix.hpp"
#include "tz/core/data/transform_hierarchy.hpp"
#include <cstdint>
#include <array>

namespace tz::ren
{
	template<unsigned int max_tex_count = 8, unsigned int max_joint4_count = 1>
	struct vertex
	{
		tz::vec3 position = tz::vec3::zero();
		float pad;
		std::array<tz::vec4, max_tex_count> texcoordn = {};
		tz::vec3 normal;
		float pad0;
		tz::vec3 tangent;
		float pad1;
		std::array<tz::vec4ui32, max_joint4_count> joint_indices = {};
		std::array<tz::vec4, max_joint4_count> joint_weights = {};
	};

	using index = std::uint32_t;

	template<unsigned int max_tex_count = 8, unsigned int max_joint4_count = 1>
	struct mesh
	{
		std::vector<vertex<max_tex_count, max_joint4_count>> vertices = {};
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

	namespace detail
	{
		struct texture_tag_t{};
		using texture_handle = tz::handle<detail::texture_tag_t>;
	}


	// represents one of the textures bound to an object (drawable)
	struct texture_locator
	{
		bool is_null() const{return this->texture == tz::nullhand;}
		// colour multiplier on the sampled texel
		tz::vec3 colour_tint = tz::vec3::filled(1.0f);
		// id within the overarching texture resource array to be sampled.
		detail::texture_handle texture = tz::nullhand;
	};

	constexpr unsigned int mesh_renderer_max_tex_count = 8;
	constexpr unsigned int mesh_renderer_max_weight_count = 4;

	// represents the data of an object (drawable).
	struct object_data
	{
		// represents the transform of the drawable, in world space.
		tz::mat4 global_transform = tz::mat4::identity();
		// extra matrix space. unused in mesh_renderer.
		tz::mat4 extra = tz::mat4::identity();
		// array of bound textures. they all do not have to be used. no indication on whether they are colour, normal map, etc...
		std::array<texture_locator, mesh_renderer_max_tex_count> bound_textures = {};
		tz::vec4ui32 extra_indices = tz::vec4ui32::zero();
	};

	/**
	* @ingroup tz_ren
	* todo: document
	* a two-pass (compute gpu command generation => render) mesh renderer. pbr?
	*/
	class mesh_renderer
	{
		struct object_tag_t{};
	public:
		mesh_renderer(unsigned int total_textures = 128);
		using vertex_t = vertex<mesh_renderer_max_tex_count, mesh_renderer_max_weight_count / 4>;
		using mesh_t = mesh<mesh_renderer_max_tex_count, mesh_renderer_max_weight_count / 4>;
		using mesh_handle = tz::handle<mesh_t>;
		using object_handle = tz::handle<object_tag_t>;
		using texture_handle = detail::texture_handle;

		struct object_init_data
		{
			tz::trs trs = {};
			mesh_handle mesh = tz::nullhand;
			std::array<texture_locator, mesh_renderer_max_tex_count> bound_textures = {};
			object_handle parent = tz::nullhand;
		};
		struct object_out_data
		{
			tz::trs local_transform;
			tz::trs global_transform;
			object_handle parent;
			std::vector<object_handle> children;
		};

		std::size_t mesh_count() const;
		std::size_t draw_count() const;
		virtual void update();
		virtual void dbgui();
		void clear();
		void clear_draws();
		mesh_handle add_mesh(mesh_t m);
		virtual object_handle add_object(object_init_data init);
		object_out_data get_object(object_handle h) const;

		void remove_object(object_handle oh);
		texture_handle add_texture(tz::vec2ui dimensions, std::span<const std::byte> image_data);
		void append_to_render_graph();
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
			render_pass_t(tz::gl::renderer_handle compute_pass, tz::gl::resource_handle compute_draw_indirect_buffer, std::string_view vertex_spirv, std::string_view fragment_spirv, unsigned int total_textures = 128);
			void dbgui();
			std::span<const object_data> get_object_datas() const;
			std::span<object_data> get_object_datas();

			tz::gl::resource_handle index_buffer = tz::nullhand;
			tz::gl::resource_handle vertex_buffer = tz::nullhand;
			tz::gl::resource_handle object_buffer = tz::nullhand;
			tz::gl::resource_handle camera_buffer = tz::nullhand;
			tz::gl::resource_handle draw_indirect_buffer_ref = tz::nullhand;
			static constexpr std::size_t extra_buffer_count = 1u;
			std::array<tz::gl::resource_handle, extra_buffer_count> extra_buffers = {tz::nullhand};
			std::vector<tz::gl::resource_handle> textures = {};
			tz::gl::renderer_handle handle = tz::nullhand;

			// not to be confused with the draw-list (part of the compute pass), which only contains the mesh locators that are gonna be drawn. this contains every mesh ever added.
			std::vector<mesh_locator> meshes = {};
			std::size_t cumulative_vertex_count = 0;
			std::size_t texture_cursor = 0;
		};

		using node_type = tz::transform_node<std::uint32_t>;

		std::optional<std::uint32_t> try_find_index_section(std::size_t index_count) const;
		std::optional<std::uint32_t> try_find_vertex_section(std::size_t vertex_count) const;
		mesh_locator add_mesh_impl(const mesh_renderer::mesh_t& m);
	protected:
		mesh_renderer(unsigned int total_textures, std::string_view vertex_spirv, std::string_view fragment_spirv);
		const mesh_locator& get_mesh_locator(mesh_handle mesh) const;
		std::span<const vertex_t> read_vertices(const mesh_locator& mloc) const;
		tz::gl::resource_handle render_pass_get_vertex_buffer_handle() const;
		object_data& get_object_data(object_handle h);
		std::size_t get_extra_buffer_count() const;
		tz::gl::resource_handle get_extra_buffer_handle(std::size_t extra_buf_id) const;

		const tz::gl::iresource& render_pass_get_resource(tz::gl::resource_handle rh) const;
		void render_pass_edit(const tz::gl::RendererEditBuilder& builder);
		void dbgui_tab_overview();
		void dbgui_tab_render();

		tz::transform_hierarchy<std::uint32_t> object_tree = {};
	private:
		compute_pass_t compute_pass = {};
		render_pass_t render_pass;
	};
}

#endif // TOPAZ_REN_MESH_HPP