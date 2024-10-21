#ifndef TOPAZ_IO_IMAGE_HPP
#define TOPAZ_IO_IMAGE_HPP
#include "tz/core/error.hpp"
#include <cstddef>
#include <span>
#include <expected>

namespace tz::io
{
	/**
	 * @ingroup tz_io
	 * @brief Describes basic information about an image.
	 *
	 * Note that all images loaded are 4-component RGBA with 1 byte ber component.
	 */
	struct image_header
	{
		/// Width of the image, in pixels.
		unsigned int width = 0u;
		/// Height of the image, in pixels.
		unsigned int height = 0u;
		/// Size of the total image data, in bytes.
		std::size_t data_size_bytes = 0u;
	};

	/**
	 * @ingroup tz_io
	 * @brief Retrieve info about an image loaded in memory
	 * @param img_file_data Data read directly from an image file. See below for the list of supported image file formats.
	 * @return A @ref image_header containing basic information about the image, or some @ref tz::error_code if the image data could not be parsed.
	 *
	 * The image file formats guaranteed are:
	 * - jpg
	 * - png
	 * - bmp
	 *
	 * Other file formats that aren't listed here might still work, but you should consider anything not on this list an implementation detail that could lose support suddenly in a new release.
	 */
	std::expected<image_header, tz::error_code> image_info(std::span<const std::byte> img_file_data);
	/**
	 * @ingroup tz_io
	 * @brief Load an image from file data resident in memory.
	 * @param img_file_data Data read directly from an image file. See the detailsof @ref image_info for a list of supported image file formats.
	 * @param buffer A buffer of memory into which decoded image data will be written. It is your responsibility to provide a buffer that is of sufficient size. To decipher the size needed, see @ref image_info.
	 * @return @ref tz::error_code::success If the image data was successfully decoded.
	 * @return @ref tz::error_code::oom If the buffer you provided was not of sufficient size.
	 * @return @ref tz::error_code::unknown_error If image decoding failed for some other reason.
	 */
	tz::error_code parse_image(std::span<const std::byte> img_file_data, std::span<std::byte> buffer);
}

#endif // TOPAZ_IO_IMAGE_HPP