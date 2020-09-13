#ifndef TOPAZ_GL_IMAGE_HPP
#define TOPAZ_GL_IMAGE_HPP
#include "gl/pixel.hpp"
#include <vector>

namespace tz::gl
{
	/**
	 * \addtogroup tz_gl Topaz Graphics Library (tz::gl)
	 * @{
	 */

	/**
	 * Represents a bitmap of some pixel type.
	 * @tparam PixelType One of the tz::gl Pixel types (see pixel.hpp)
	 */
	template<class PixelType>
	class Image
	{
	public:
		/**
		 * Construct an image with the given dimensions.
		 * @param width Number of pixels per row.
		 * @param height Number of pixels per column.
		 */
		Image(unsigned int width, unsigned int height);
		/**
		 * Retrieve the width of the image, in pixels.
		 * @return Width in pixels.
		 */
		unsigned int get_width() const;
		/**
		 * Retrieve the height of the image, in pixels.
		 * @return Height in pixels.
		 */
		unsigned int get_height() const;
		/**
		 * Retrieve the OpenGL pixel type corresponding to the pixel type of this image.
		 * @return OpenGL enum corresponding to image pixel.
		 */
		GLenum get_pixel_type() const;
		/**
		 * Retrieve image data. Size is guaranteed to be equal to this->get-width() * this->get_height().
		 * @return Contiguous array of image data.
		 */
		const PixelType* data() const;
		/**
		 * Retrieve image data. Size is guaranteed to be equal to this->get-width() * this->get_height().
		 * @return Contiguous array of image data.
		 */
		PixelType* data();
		/**
		 * Retrieve the pixel at the given position in the bitmap.
		 * Note that (0, 0) corresponds to the top-left of the pixel.
		 * Precondition: x < this->get_width() && y < this->get_height(). Otherwise, this will assert and invoke UB.
		 * @param x Horizontal co-ordinate of the desired pixel.
		 * @param y Vertical co-ordinate of the desired pixel.
		 * @return Reference to the pixel at the given position.
		 */
		const PixelType& operator()(unsigned int x, unsigned int y) const;
		/**
		 * Retrieve the pixel at the given position in the bitmap.
		 * Note that (0, 0) corresponds to the top-left of the pixel.
		 * Precondition: x < this->get_width() && y < this->get_height(). Otherwise, this will assert and invoke UB.
		 * @param x Horizontal co-ordinate of the desired pixel.
		 * @param y Vertical co-ordinate of the desired pixel.
		 * @return Reference to the pixel at the given position.
		 */
		PixelType& operator()(unsigned int x, unsigned int y);
		bool operator==(const Image<PixelType>& rhs) const;
		bool operator!=(const Image<PixelType>& rhs) const;
	private:
		/// Width, in pixels.
		unsigned int width;
		/// Height, in pixels.
		unsigned int height;
		// Represented similarly to a row-major matrix.
		std::vector<PixelType> pixel_data;
	};

	/**
	 * @}
	 */
}

#include "gl/image.inl"
#endif // TOPAZ_GL_IMAGE_HPP