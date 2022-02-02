#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
#if TZ_OGL
#include "core/tz.hpp"
#include "glad/glad.h"

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl2_graphicsapi_ogl
	 * @defgroup tz_gl_ogl2 OpenGL Backend
	 * Documentation for the OpenGL Backend, which acts as a large abstraction around the OpenGL API. This is a low-level module free of all context relevant to Topaz.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * Initialise the OpenGL backend.
	 */
	void initialise(tz::GameInfo game_info, tz::ApplicationType app_type);
	void terminate();

	constexpr tz::Version ogl_version{4, 6, 0};
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
