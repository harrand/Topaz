#ifndef TOPAZ_REN_MESH_HPP
#define TOPAZ_REN_MESH_HPP
#include "tz/gl/renderer.hpp"
#include "tz/core/data/vector.hpp"
#include "tz/core/matrix.hpp"
#include "tz/io/gltf.hpp"
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
		tz::vec3 normal = tz::vec3::zero();
		float pad0;
		tz::vec3 tangent = tz::vec3::zero();
		float pad1;
		std::array<tz::vec4ui32, max_joint4_count> joint_indices = {};
		std::array<tz::vec4, max_joint4_count> joint_weights = {};
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

	namespace detail
	{
		struct texture_tag_t{};
	}

	using texture_handle = tz::handle<detail::texture_tag_t>;

	// represents one of the textures bound to an object (drawable)
	struct texture_locator
	{
		bool is_null() const{return this->texture == tz::nullhand;}
		// colour multiplier on the sampled texel
		tz::vec3 colour_tint = tz::vec3::filled(1.0f);
		// id within the overarching texture resource array to be sampled.
		texture_handle texture = tz::nullhand;
	};

	constexpr unsigned int mesh_renderer_max_tex_count = 2;
	constexpr unsigned int mesh_renderer_max_joint4_count = 1;

	// represents the data of an object (drawable).
	struct object_data
	{
		// represents the transform of the drawable, in world space.
		tz::mat4 model = tz::mat4::identity();
		tz::mat4 global_transform = tz::mat4::identity();
		// array of bound textures. they all do not have to be used. no indication on whether they are colour, normal map, etc...
		std::array<texture_locator, mesh_renderer_max_tex_count> bound_textures = {};
		std::uint32_t parent = static_cast<std::uint32_t>(-1);
		float pad0[3];
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
		using vertex_t = vertex<mesh_renderer_max_tex_count, mesh_renderer_max_joint4_count>;
		using mesh_t = mesh<mesh_renderer_max_tex_count>;
		using mesh_handle = tz::handle<mesh_t>;
		using object_handle = tz::handle<object_tag_t>;
		using texture_handle = texture_handle;

		struct stored_assets
		{
			std::vector<mesh_handle> meshes = {};
			std::vector<texture_handle> textures = {};
			std::vector<object_handle> objects = {};
		};

		std::size_t mesh_count() const;
		std::size_t draw_count() const;
		void clear();
		void clear_draws();
		mesh_handle add_mesh(mesh_t m);
		object_handle add_object(mesh_handle m, object_data data = {});
		texture_handle add_texture(tz::vec2ui dimensions, std::span<const std::byte> image_data);
		stored_assets add_gltf(const tz::io::gltf& gltf);
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
			std::span<const object_data> get_object_datas() const;
			std::span<object_data> get_object_datas();

			tz::gl::resource_handle index_buffer = tz::nullhand;
			tz::gl::resource_handle vertex_buffer = tz::nullhand;
			tz::gl::resource_handle object_buffer = tz::nullhand;
			tz::gl::resource_handle camera_buffer = tz::nullhand;
			tz::gl::resource_handle draw_indirect_buffer_ref = tz::nullhand;
			std::vector<tz::gl::resource_handle> textures = {};
			tz::gl::renderer_handle handle = tz::nullhand;

			// not to be confused with the draw-list (part of the compute pass), which only contains the mesh locators that are gonna be drawn. this contains every mesh ever added.
			std::vector<mesh_locator> meshes = {};
			std::size_t cumulative_vertex_count = 0;
			std::size_t texture_cursor = 0;
		};

		std::optional<std::uint32_t> try_find_index_section(std::size_t index_count) const;
		std::optional<std::uint32_t> try_find_vertex_section(std::size_t vertex_count) const;
		mesh_locator add_mesh_impl(const mesh_renderer::mesh_t& m);
		void dbgui_impl();
		stored_assets add_gltf_impl(const tz::io::gltf& gltf);
		void impl_expand_gltf_node(const tz::io::gltf& gltf, const tz::io::gltf_node& node, stored_assets& assets, std::span<std::size_t> mesh_submesh_indices, std::span<std::optional<tz::io::gltf_material>> submesh_textures, std::uint32_t parent = static_cast<std::uint32_t>(-1));
		tz::mat4 compute_global_transform(std::uint32_t obj_id) const;
		void compute_global_transforms();

		compute_pass_t compute_pass = {};
		render_pass_t render_pass;
	};
}

#endif // TOPAZ_REN_MESH_HPP