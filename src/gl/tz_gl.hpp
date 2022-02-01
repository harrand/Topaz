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
	 * @defgroup tz_gl2_renderer Renderers
	 * Documentation for Renderers.
	 */

	/**
	 * @ingroup tz_gl2
	 * @defgroup tz_gl2_graphicsapi Graphics API Specifics
	 * Documentation for implementation layers for the supported graphics APIs.
	 */
	
		/**
		 * @ingroup tz_gl2_graphicsapi
		 * @defgroup tz_gl2_graphicsapi_vk Vulkan
		 * Documentation for all things Vulkan.
		 */

			/**
			 * @ingroup tz_gl2_graphicsapi_vk
			 * @defgroup tz_gl2_graphicsapi_vk_frontend Vulkan Frontend
			 * Documentation for the Vulkan Frontend, which connects the Vulkan Backend to the tz::gl2 API.
			 */

		/**
		 * @ingroup tz_gl2_graphicsapi
		 * @defgroup tz_gl2_graphicsapi_ogl OpenGL
		 * Documentation for all things OpenGL.
		 */

			/**
			 * @ingroup tz_gl2_graphicsapi_ogl
			 * @defgroup tz_gl2_graphicsapi_ogl_frontend OpenGL Frontend
			 * Documentation for the OpenGL Frontend, which connects the OpenGL Backend to the tz::gl2 API.
			 */
	
			/**
			 * @ingroup tz_gl2_graphicsapi_ogl
			 * @defgroup tz_gl2_graphicsapi_ogl_backend OpenGL Backend
			 * Documentation for the OpenGL Backend, which acts as a large abstraction around the OpenGL API. This is a low-level module free of all context relevant to Topaz.
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
