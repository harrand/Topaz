#include "utility/log.hpp"
#include "SDL2/SDL_ttf.h"
#include "graphics/font.hpp"

#ifndef topaz_assert
#define topaz_assert(x, y)
#endif

namespace tz::graphics
{
	inline void initialise()
	{
		topaz_assert(tz::graphics::has_context, "tz::graphics::initialise(): Error: Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.");
		tz::debug::print("tz::graphics::initialise(): OpenGL context detected, initialising tz::graphics...\n");
		[[maybe_unused]] GLenum status = glewInit();
		topaz_assert(status == GLEW_OK, "tz::graphics::initialise(): Fatal Error: Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!");
		tz::debug::print("tz::graphics::initialise(): Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).\n");
		//[[maybe_unused]] int ttf_init_result = TTF_Init();
		//topaz_assert(ttf_init_result == 0, "tz::graphics::initialise(): Fatal Error: Initialisation of SDL2_ttf failed.");
		auto error = FT_Init_FreeType(&tz::graphics::detail::freetype_library);
		topaz_assert(error == 0, "tz::graphics::initialize(): Fatal Error: Initializer of FreeType failed.");
		tz::graphics::detail::freetype_initialized = true;
		tz::debug::print("tz::graphics::initialise(): Successfully initialised SDL2_ttf.\n");
		tz::graphics::initialised = true;
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
	}
	
	inline void terminate()
	{
		FT_Done_FreeType(tz::graphics::detail::freetype_library);
		tz::graphics::detail::freetype_initialized = false;
		tz::debug::print("tz::graphics::terminate(): Terminated SDL2_ttf\n");
		tz::debug::print("tz::graphics::terminate(): Terminated tz::graphics via GLEW (OpenGL).\n");
	}

	inline void scene_render_mode()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_CLAMP);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	inline void gui_render_mode()
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_CULL_FACE);
	}

	inline void enable_wireframe_render(bool wireframes, float wireframe_width)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframes ? GL_LINE : GL_FILL);
		if(wireframes)
			glLineWidth(wireframes ? wireframe_width : 1.0f);
	}
}