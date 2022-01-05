#ifndef TOPAZ_GL2_TZ_GL_HPP
#define TOPAZ_GL2_TZ_GL_HPP
#include "core/version.hpp"
#include <array>

namespace tz::gl2
{
	/**
	 * @defgroup tz_gl2 Topaz Graphics Library 2 (tz::gl2)
	 * Documentation for the Topaz Graphics Library (since 3.1).
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_io Inputs and Outputs
	 * Documentation for inputs and outputs for both Renderers and Processors.
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_res Resources
	 * Documentation for Renderer and Processor resources. Namely, buffers and images.
	 */
	
	/**
	 * @ingroup tz_gl2
	 * Topaz is built against exactly one graphics API, having a low-level graphics backedn for each supported API. This enum specifies all the supported graphics APIs with a corresponding render backend.
	 */
	enum class RenderBackend
	{
		OpenGL,
		Vulkan
	};
}

#endif // TOPAZ_GL2_TZ_GL_HPP
