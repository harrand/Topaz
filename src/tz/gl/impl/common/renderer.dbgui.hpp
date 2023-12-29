#include "tz/gl/api/renderer.hpp"
#include "tz/gl/api/shader.hpp"
#include "imgui.h"
#undef assert

namespace tz::gl
{
	/**
	 * @ingroup tz_gl2_renderer
	 * Helper function which displays render-api-agnostic information about renderers.
	 */
	void common_renderer_dbgui(renderer_type auto& renderer);
}