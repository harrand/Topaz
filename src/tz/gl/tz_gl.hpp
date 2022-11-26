#ifndef TOPAZ_GL2_TZ_GL_HPP
#define TOPAZ_GL2_TZ_GL_HPP
#include "hdk/data/version.hpp"
#include <array>

namespace tz::gl
{
	/**
	 * @defgroup tz_gl2 Graphics Library (tz::gl)
	 * Hardware-accelerated 3D graphics library.
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

	/**
	 * @ingroup tz_gl2
	 * Topaz is built against exactly one graphics API, having a low-level graphics backedn for each supported API. This enum specifies all the supported graphics APIs with a corresponding render backend.
	 */
	enum class RenderBackend
	{
		OpenGL,
		Vulkan
	};

	void initialise();
	void terminate();
}

#endif // TOPAZ_GL2_TZ_GL_HPP
