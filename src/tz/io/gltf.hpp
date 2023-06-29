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
		std::vector<std::byte> data;
	};

	class gltf
	{
	public:
		gltf() = default;
		static gltf from_memory(std::string_view sv);
	private:
		gltf(json data);
		void parse_resources();
		gltf_resource load_buffer(json node);
		
		std::size_t parse_data_uri(std::string_view data_uri, std::span<std::byte> byte_view) const;

		json data = {};
		std::vector<gltf_resource> resources = {};
		std::size_t parsed_buf_count = 0;
		std::size_t parsed_img_count = 0;
	};
}
