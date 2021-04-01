#ifndef TOPAZ_ALGO_IMAGE_PROCESSING_HPP
#define TOPAZ_ALGO_IMAGE_PROCESSING_HPP
#include "gl/image.hpp"

namespace tz::algo
{
    /**
	 * \addtogroup tz_algo Topaz Algorithms Library (tz::algo)
	 * Contains common algorithms used in Topaz modules that aren't available in the C++ standard library.
	 * @{
	 */
    enum class ImageFlipDirection
    {
        Horizontal,
        Vertical
    };

    template<tz::gl::PixelType Pixel>
    void flip_image(tz::gl::Image<Pixel>& image, ImageFlipDirection direction);

    /**
     * @}
     */
}

#include "algo/image_processing.inl"
#endif