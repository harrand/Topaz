#ifndef TOPAZ_GL2_TZ_GL_HPP
#define TOPAZ_GL2_TZ_GL_HPP
#include "tz/core/data/version.hpp"
#include "tz/lua/api.hpp"
#include <array>

namespace tz::gl
{
	/**
	 * @ingroup tz_cpp
	 * @defgroup tz_gl2 Graphics Library
	 * Low-level 3D graphics library.
	 *
	 * Rendering 3D graphics in Topaz is achieved by doing the following:
	 * - Populate a @ref tz::gl::renderer_info describing how to render what you want.
	 * - Pass the info to `tz::gl::device::create_renderer()` via the global @ref tz::gl::get_device(). Save the returned renderer handle as a local variable.
	 * - Every frame, use the global @ref tz::gl::get_device() via `tz::gl::device::get_renderer(handle)`, to invoke invoke `tz::gl::renderer::render()` to draw your graphics.
	 *   - Don't forget to @ref tz::begin_frame() and @ref tz::end_frame()!
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_io Inputs and Outputs
	 * Documentation for inputs and outputs for both Renderers and Processors.
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_res Resources
	 * <a href="https://github.com/harrand/Topaz/wiki/Renderer-Resources">Resources Wiki</a> 
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_renderer Renderers
	 * Documentation for Renderers.
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_graphicsapi Maintainer Guide
	 * For Topaz maintainers only.
	 *
	 * Here you will find implementation details for both of the graphics API backends (@ref tz_gl2_graphicsapi_ogl and @ref tz_gl2_graphicsapi_vk)
	 */
	
		/**
		 * @ingroup tz_gl2_graphicsapi
		 * @defgroup tz_gl2_graphicsapi_vk Vulkan
		 * Documentation for all things Vulkan.
		 */

			/**
			 * @ingroup tz_gl2_graphicsapi_vk
			 * @defgroup tz_gl2_graphicsapi_vk_frontend Vulkan Frontend
			 * Documentation for the Vulkan Frontend, which connects the Vulkan Backend to the tz::gl API.
			 */

		/**
		 * @ingroup tz_gl2_graphicsapi
		 * @defgroup tz_gl2_graphicsapi_ogl OpenGL
		 * Documentation for all things OpenGL.
		 */

			/**
			 * @ingroup tz_gl2_graphicsapi_ogl
			 * @defgroup tz_gl2_graphicsapi_ogl_frontend OpenGL Frontend
			 * Documentation for the OpenGL Frontend, which connects the OpenGL Backend to the tz::gl API.
			 */

	void initialise();
	void terminate();

	void lua_initialise(tz::lua::state& state);
}

#endif // TOPAZ_GL2_TZ_GL_HPP
