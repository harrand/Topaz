#include "tz/io/image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstring>
#include <span>

namespace tz::io
{
	image image::load_from_memory(std::string_view img_file_data)
	{
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
}
