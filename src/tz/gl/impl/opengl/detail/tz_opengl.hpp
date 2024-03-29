#ifndef TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
#define TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
#if TZ_OGL
#include "tz/tz.hpp"
#include "glad/glad.h"
#if TZ_PROFILE
#include "tracy/TracyOpenGL.hpp"
#endif // TZ_PROFILE

namespace tz::gl::ogl2
{
	/**
	 * @ingroup tz_gl2_graphicsapi_ogl
	 * @defgroup tz_gl_ogl2 OpenGL Backend
	 * Documentation for the OpenGL backend, which acts as a large abstraction around OpenGL. This is a low-level module free of all context relevant to Topaz.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * @defgroup tz_gl_ogl2_buffers Buffers
	 * Documentation for OpenGL Buffers.
	 */

	/**
	 * @ingroup tz_gl_ogl2
	 * @defgroup tz_gl_ogl2_image Images, Samplers and Formats
	 * Documentation for everything relating to Images, Samplers and image_formats.
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

	constexpr tz::version ogl_version{4, 6, 0, tz::version_type::normal};
	using ogl_string = std::basic_string<GLchar>;
	using ogl_string_view = std::basic_string_view<GLchar>;

	bool supports_bindless_textures();
}

#endif // TZ_OGL
#endif // TOPAZ_GL_IMPL_BACKEND_OGL2_TZ_OPENGL_HPP
