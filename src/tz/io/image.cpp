#include "tz/io/image.hpp"
#include "tz/topaz.hpp"
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace tz::io
{
	std::expected<image_header, tz::error_code> image_info(std::span<const std::byte> img_file_data)
	{
		int w, h, channels;
		int ok = stbi_info_from_memory(reinterpret_cast<const stbi_uc*>(img_file_data.data()), img_file_data.size_bytes(), &w, &h, &channels);
		if(ok != 1)
		{
			UNERR(tz::error_code::precondition_failure, "bad image file data: {}", stbi_failure_reason());
		}

		return image_header
		{
			.width = static_cast<unsigned int>(w),
			.height = static_cast<unsigned int>(h),
			.data_size_bytes = w * h * 4u * 1u
		};
	}

	tz::error_code parse_image(std::span<const std::byte> img_file_data, std::span<std::byte> buffer)
	{
		int w, h, channels;
		stbi_uc* imgdata = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(img_file_data.data()), img_file_data.size_bytes(), &w, &h, &channels, 4);
		std::size_t real_image_size = w * h * 4u * 1u;
		if(buffer.size_bytes() < real_image_size)
		{
			RETERR(tz::error_code::oom, "buffer provided to parse_image was too small ({} bytes), it needs to be at least {} bytes", buffer.size_bytes(), real_image_size);
		}
		if(imgdata == nullptr)
		{
			RETERR(tz::error_code::unknown_error, "unknown error occurred during image parsing: {}", stbi_failure_reason());
		}
		std::memcpy(buffer.data(), imgdata, real_image_size);
		return tz::error_code::success;
	}
}