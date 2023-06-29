#include "nlohmann/json.hpp"
#undef assert

namespace tz::io
{
	using json = nlohmann::json;

	enum class gltf_resource_type
	{
		buffer,
		image
	};

	struct gltf_resource
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
		std::size_t version = -1;
		std::size_t size = -1;
	};

	enum class gltf_buffer_view_type
	{
		vertex = 34962,
		index = 34963
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

	struct gltf_scene
	{

	};

	class gltf
	{
	public:
		gltf() = default;
		// note: you should pass in the file contents of a .glb.
		static gltf from_memory(std::string_view sv);
		std::span<const std::byte> view_buffer(gltf_buffer_view view) const;
	private:
		gltf(std::string_view glb_data);
		void parse_header(std::string_view header);
		void parse_chunks(std::string_view chunkdata);
		void load_resources();
		void create_views();
		gltf_resource load_buffer(json node);
		std::span<const std::byte> get_binary_data(std::size_t offset, std::size_t len) const;

		gltf_header header;
		std::vector<gltf_chunk_data> chunks = {};
		json data = {};
		std::vector<gltf_resource> resources = {};
		std::vector<gltf_buffer_view> views = {};
		std::size_t parsed_buf_count = 0;
		std::size_t parsed_img_count = 0;
	};
}
