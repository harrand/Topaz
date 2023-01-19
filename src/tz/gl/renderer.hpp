#ifndef TOPAZ_GL2_RENDERER_HPP
#define TOPAZ_GL2_RENDERER_HPP

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/renderer.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/renderer.hpp"
#endif

namespace tz::gl
{

	/**
	 * @ingroup tz_gl2_renderer
	 * @class renderer
	 * Implementation of @ref tz::gl::renderer_type.
	 */
	#if TZ_VULKAN
		using renderer = renderer_vulkan;
	#elif TZ_OGL
		using renderer = renderer_ogl;
	#endif
}

#endif // TOPAZ_GL2_RENDERER_HPP
