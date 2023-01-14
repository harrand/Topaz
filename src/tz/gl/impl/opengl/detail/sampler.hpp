#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_SAMPLER_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_SAMPLER_HPP
#if TZ_OGL
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl_ogl2_image 
	 * Determines how sub-pixel colours are filtered.
	 */
	enum class LookupFilter : GLint
	{
		/// - Texel nearest (via Manhattan Distance) to the texture coordinate is used.
		Nearest = GL_NEAREST,
		/// - Weighted average of four closest texels to the texture coordinate.
		Linear = GL_LINEAR
	};

	/**
	 * @ingroup tz_gl_ogl2_image 
	 * Determines the value retrieved from a texture lookup if the texture coordinate is out of range.
	 */
	enum class AddressMode : GLint
	{
		/// - An imaginary line is drawn from the out-of-bounds-coordinate back to the edge of the sampled image. The colour of the texel that it meets is used.
		ClampToEdge = GL_CLAMP_TO_EDGE,
		/// - The texcoord is essentially modulo'd with the image dimensions.
		Repeat = GL_REPEAT,
		/// - Just like Repeat, except mirrored.
		MirroredRepeat = GL_MIRRORED_REPEAT
		
	};

	/**
	 * @ingroup tz_gl_ogl2_image 
	 * Describes various details about texture lookups from a sampled image.
	 */
	struct Sampler
	{
		/// Filter used when texels are larger than screen pixels.
		LookupFilter min_filter;
		/// Filter used when texels are smaller than screen pixels.
		LookupFilter mag_filter;

		/// Addressing mode used when out of range on the s (x) axis.
		AddressMode address_mode_s;
		/// Addressing mode used when out of range on the t (y) axis.
		AddressMode address_mode_t;
		/// Addressing mode used when out of range on the r (z) axis. When not in the dark ages this would be named 'p', such as in OGL GLSL.
		AddressMode address_mode_r;
	};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_SAMPLER_HPP
