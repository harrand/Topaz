#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
#if TZ_OGL
#include "tz/core/tz.hpp"
#include "glad/glad.h"
#if TZ_PROFILE
#include "TracyOpenGL.hpp"
#endif // TZ_PROFILE

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl2_graphicsapi_ogl
	 * @defgroup tz_gl_ogl2 OpenGL Backend
	 * Documentation for the OpenGL Backend (4.6 Desktop), which acts as a large abstraction around the OpenGL API. This is a low-level module free of all context relevant to Topaz.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * @defgroup tz_gl_ogl2_buffers Buffers
	 * Documentation for OpenGL Buffers.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * @defgroup tz_gl_ogl2_image Images, Samplers and Formats
	 * Documentation for everything relating to Images, Samplers and ImageFormats.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * @defgroup tz_gl_ogl2_shader Shaders
	 * Documentation for everything relating to shaders.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * @defgroup tz_gl_ogl2_framebuffer Framebuffers
	 * Documentation for everything relating to framebuffers.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * Initialise the OpenGL backend.
	 */
	void initialise();
	/**
	 * @ingroup tz_gl_ogl2
	 * Terminate the OpenGL backend.
	 */
	void terminate();
	/**
	 * @ingroup tz_gl_ogl2
	 * Query as to whether the OpenGL backend has been initialised or not.
	 */
	bool is_initialised();

	constexpr tz::Version ogl_version{4, 5, 0};
	using OGLString = std::basic_string<GLchar>;
	using OGLStringView = std::basic_string_view<GLchar>;

	bool supports_bindless_textures();
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
