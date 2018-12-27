#include "utility/log.hpp"

namespace tz::graphics
{
	inline void initialise()
	{
		if(!tz::graphics::has_context)
		{
			tz::debug::print("Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.\n");
			return;
		}
		tz::debug::print("OpenGL context detected, initialising tz::graphics...\n");
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			tz::debug::print("Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!\n");
            //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Topaz Error", (std::string("Topaz graphics module (tz::graphics) failed to initialise.\nError message:\n ") + std::string(SDL_GetError())).c_str(), NULL);
        }
		else
			tz::debug::print("Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).\n");
		if(TTF_Init() == 0)
			tz::debug::print("Successfully initialised SDL2_ttf.\n");
		else
			tz::debug::print("Initialisation of SDL2_ttf failed.\n");
		tz::graphics::initialised = true;
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
	}
	
	inline void terminate()
	{
		TTF_Quit();
		tz::debug::print("Terminated SDL2_ttf\n");
		tz::debug::print("Terminated tz::graphics via GLEW (OpenGL).\n");
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

	inline void enable_wireframe_render(bool wireframes)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframes ? GL_LINE : GL_FILL);
	}
}