#include "tz/io/image.hpp"
#include "tz/core/profile.hpp"
#include "tz/core/debug.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <cstring>
#include <span>
#undef assert

namespace tz::io
{
	image image::load_from_memory(std::string_view img_file_data)
	{
		TZ_PROFZONE("image - load from memory", 0xFFFF2222);
		int w, h, channels;
		stbi_uc* imgdata = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(img_file_data.data()), img_file_data.size(), &w, &h, &channels, 4);
		std::span<stbi_uc> imgdata_span{imgdata, static_cast<std::size_t>(w * h * 4)};

		image ret;
		ret.width = w;
		ret.height = h;
		ret.data.resize(w * h * 4);
		std::memcpy(ret.data.data(), imgdata, w * h * 4);

		stbi_image_free(imgdata);
		return ret;
	}

	image image::load_from_file(std::string_view path)
	{
		std::ifstream file(path.data(), std::ios::binary);
		tz::assert(file.good(), "Could not load image from file because the path was wrong, or something else went wrong.");
		std::string buffer(std::istreambuf_iterator<char>(file), {});
		return image::load_from_memory(buffer);
	}
}
