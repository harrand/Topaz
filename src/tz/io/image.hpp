#ifndef TOPAZ_IO_IMAGE_HPP
#define TOPAZ_IO_IMAGE_HPP
#include <vector>
#include <cstddef>
#include <string_view>

namespace tz::io
{
	// all images are loaded as rgba32.
	struct image
	{
		unsigned int width;
		unsigned int height;
		std::vector<std::byte> data;

		static image load_from_memory(std::string_view img_filedata);
		static image load_from_file(std::string_view path);
	};
}

#endif // TOPAZ_IO_IMAGE_HPP
