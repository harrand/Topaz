#include "header_export.hpp"
#include "source_transform.hpp"

namespace tzslc
{
	std::string std_bytenise(std::string_view binary_data)
	{
		std::string res;
		for(std::size_t i = 0; i < binary_data.size(); i++)
		{
			res += std::to_string(static_cast<std::int8_t>(binary_data[i]));
			if(i != binary_data.size() - 1)
			{
				res += ",";
			}
		}
		return res;
	}

	bool export_header(std::string_view filename, std::string& buffer)
	{
		// filename is a full path, matching "root/path_parent/filename.vertex.tzsl.spv" for vulkan, and replace ".spv" with ".glsl" for ogl. We want just: "filename_vertex. We'll use a regex for this."
		std::string filename_cpy{filename};
		const char* regex_str;
		#if TZ_VULKAN
			regex_str = ".*[\\/\\\\]([a-zA-Z_]+)\\.([a-zA-Z]+)\\.tzsl\\.spv";
		#elif TZ_OGL
			regex_str = ".*[\\/\\\\]([a-zA-Z_]+)\\.([a-zA-Z]+)\\.tzsl\\.glsl";
		#else
			static_assert(false);
		#endif
		tzslc::transform(filename_cpy, std::regex{regex_str}, [&](auto beg, auto end)->std::string
		{
			//tz_assert(std::distance(beg, end) == 1, "Failed to parse shader file name.");
			std::string part_we_want = *beg;
			std::string shader_type_name = *(beg + 1);
			return part_we_want + "_" + shader_type_name;
		});
		// Note, filename_cpy might be a spirv binary which is not a valid source file. We should convert it all into a series of std::bytes. This will not be fun.
		std::size_t byte_count = buffer.size();
		std::string buffer_array_literal = std_bytenise(buffer);
		buffer = std::string("#include <cstdint>\n#include<span>\n/*tzslc_gen_header*/constexpr std::array<std::int8_t, " + std::to_string(byte_count) + "> " + filename_cpy + "{") + buffer_array_literal + "};";
		return false;
	}
}
