#ifndef TOPAZ_IO_GLTF_HPP
#define TOPAZ_IO_GLTF_HPP
#include "tz/core/data/enum_field.hpp"
#include "tz/core/data/vector.hpp"
#include "tz/core/matrix.hpp"
#include "tz/io/image.hpp"
#include "nlohmann/json.hpp"
#undef assert

namespace tz::io
{
	namespace detail
	{
		// i hate msvc so damn much.
		constexpr std::size_t badzu = static_cast<std::size_t>(-1);
	}
	using json = nlohmann::json;

	struct gltf_buffer
	{
		std::string label;
		std::span<const std::byte> data;
	};

	enum class gltf_chunk_type
	{
		json,
		bin,
		ext
	};

	struct gltf_chunk_data
	{
		gltf_chunk_type type;
		std::vector<std::byte> data;
	};

	struct gltf_header
	{
		std::size_t version = detail::badzu;
		std::size_t size = detail::badzu;
	};

	struct gltf_scene
	{
		std::vector<std::size_t> nodes = {};
		std::string name = "Unnamed Scene";
	};

	struct gltf_node
	{
		std::string name = "Unnamed Node";
		std::size_t mesh = detail::badzu;
		std::size_t skin = detail::badzu;
		tz::mat4 transform = tz::mat4::identity();
		std::vector<std::size_t> children = {};
	};

	enum class gltf_buffer_view_type
	{
		none = 0,
		vertex = 34962,
		index = 34963,
	};

	struct gltf_buffer_view
	{
		std::size_t buffer_id;
		gltf_buffer_view_type type;
		std::size_t offset;
		std::size_t length;
	};

	enum class gltf_accessor_component_type
	{
		sbyte = 5120,
		ubyte = 5121,
		sshort = 5122,
		ushort = 5123,
		uint = 5125,
		flt = 5126
	};

	enum class gltf_accessor_type
	{
		scalar,
		vec2,
		vec3,
		vec4,
		mat2,
		mat3,
		mat4
	};

	struct gltf_accessor
	{
		std::size_t buffer_view_id;
		gltf_accessor_component_type component_type;
		std::size_t element_count;
		gltf_accessor_type type;
	};

	constexpr int gltf_max_texcoord_attribs = 8;
	constexpr int gltf_max_color_attribs = 8;
	constexpr int gltf_max_joint_attribs = 8;
	constexpr int gltf_max_weight_attribs = 8;
	enum class gltf_attribute
	{
		position,
		normal,
		tangent,
		texcoord0,
		texcoord_max = texcoord0 + gltf_max_texcoord_attribs,
		color0,
		colormax = color0 + gltf_max_color_attribs,
		joint0,
		jointmax = joint0 + gltf_max_joint_attribs,
		weight0,
		weightmax = weight0 + gltf_max_weight_attribs,
		_count
	};
	using accessor_ref = std::array<int, static_cast<int>(gltf_attribute::_count)>;

	using gltf_attributes = tz::enum_field<gltf_attribute>;

	enum class gltf_primitive_mode
	{
		points,
		lines,
		line_loop,
		line_strip,
		triangles,
		triangle_strip,
		triangle_fan
	};

	struct gltf_mesh
	{
		std::string name = "Untitled Mesh";
		struct submesh
		{
			gltf_attributes attributes = {};
			accessor_ref accessors = {};
			std::size_t indices_accessor = detail::badzu;
			std::size_t material_id = detail::badzu;
			gltf_primitive_mode mode;

		};
		std::vector<submesh> submeshes = {};
	};

	struct gltf_skin
	{
		std::size_t inverse_bind_matrix_accessor_id = detail::badzu;
		std::size_t skeleton_id = detail::badzu;
		std::vector<std::size_t> joints = {};
		std::string name = "Unnamed Skin";
	};

	enum class gltf_animation_channel_target_path
	{
		translation,
		rotation,
		scale,
		weights
	};

	struct gltf_animation_channel_target
	{
		std::size_t node;
		gltf_animation_channel_target_path path;
	};

	struct gltf_animation_channel
	{
		std::size_t sampler_id;
		gltf_animation_channel_target target;
	};

	enum class gltf_animation_key_interpolation
	{
		linear,
		step,
		cubic_spline
	};

	struct gltf_animation_sampler
	{
		float input;
		float output;
		gltf_animation_key_interpolation interpolation;
	};

	struct gltf_animation
	{
		std::string name = "Unnamed";
		std::vector<gltf_animation_channel> channels = {};
		std::vector<gltf_animation_sampler> samplers = {};
	};

	enum class gltf_image_type
	{
		png,
		jpg
	};

	struct gltf_image
	{
		std::string name;
		gltf_image_type type;
		std::size_t buffer_view_id;
	};

	struct gltf_material
	{
		std::string name = "Null Material";
		std::size_t color_texture_id = detail::badzu;
		std::size_t color_texcoord_id = detail::badzu;
		std::size_t normal_texture_id = detail::badzu;
		std::size_t normal_texcoord_id = detail::badzu;
		float normal_scale = 1.0f;
		std::size_t occlusion_texture_id = detail::badzu;
		std::size_t occlusion_texcoord_id = detail::badzu;
		float occlusion_strength = 1.0f;
		std::size_t emissive_texture_id = detail::badzu;
		std::size_t emissive_texcoord_id = detail::badzu;
	};

	struct gltf_vertex_data
	{
		tz::vec3 position;
		tz::vec3 normal;
		tz::vec3 tangent;
		std::array<tz::vec2, gltf_max_texcoord_attribs> texcoordn;
		std::array<tz::vec3, gltf_max_color_attribs> colorn;
		// TODO: joints
		// TODO: weights
	};

	enum class gltf_submesh_texture_type
	{
		color,
		normal,
		occlusion,
		emissive
	};
;
	struct gltf_submesh_texture_data
	{
		std::size_t texcoord_id;
		std::size_t image_id;
		float extra_data = 1.0f; // normal scale / occlusion strength.
		gltf_submesh_texture_type type;
	};

	struct gltf_submesh_data
	{
		std::string name;
		gltf_attributes attributes = {};
		std::vector<gltf_vertex_data> vertices = {};
		std::vector<std::uint32_t> indices = {};
		std::vector<gltf_submesh_texture_data> textures = {};
	};

	class gltf
	{
	public:
		gltf() = default;
		// note: you should pass in the file contents of a .glb.
		static gltf from_memory(std::string_view sv);
		static gltf from_file(const char* path);
		std::span<const std::byte> view_buffer(gltf_buffer_view view) const;
		std::span<const gltf_mesh> get_meshes() const;
		std::span<const gltf_buffer> get_buffers() const;
		std::span<const gltf_image> get_images() const;
		std::span<const gltf_node> get_nodes() const;
		std::vector<gltf_node> get_active_nodes() const;
		gltf_submesh_data get_submesh_vertex_data(std::size_t meshid, std::size_t submeshid) const;
		tz::io::image get_image_data(std::size_t imageid) const;
	private:
		gltf(std::string_view glb_data);
		void parse_header(std::string_view header);
		void parse_chunks(std::string_view chunkdata);
		void create_scenes();
		void create_nodes();
		void create_buffers();
		void create_images();
		void create_materials();
		void create_skins();
		void create_animations();
		void create_views();
		void create_accessors();
		void create_meshes();
		gltf_buffer load_buffer(json node);
		gltf_mesh load_mesh(json node);
		std::span<const std::byte> get_binary_data(std::size_t offset, std::size_t len) const;

		gltf_header header;
		std::vector<gltf_chunk_data> chunks = {};
		json data = {};
		std::size_t active_scene_id = detail::badzu;
		std::vector<gltf_scene> scenes = {};
		std::vector<gltf_node> nodes = {};
		std::vector<gltf_buffer> buffers = {};
		std::vector<gltf_image> images = {};
		std::vector<gltf_material> materials = {};
		std::vector<gltf_skin> skins = {};
		std::vector<gltf_animation> animations = {};
		std::vector<gltf_buffer_view> views = {};
		std::vector<gltf_accessor> accessors = {};
		std::vector<gltf_mesh> meshes = {};
		std::size_t parsed_buf_count = 0;
	};
}
#endif // TOPAZ_IO_GLTF_HPP
