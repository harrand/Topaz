namespace tz::graphics
{
	inline void initialise(bool print_progress)
	{
		if(!tz::graphics::has_context)
		{
			if(print_progress)
				tz::util::log::warning("Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.");
			return;
		}
		if(print_progress)
			tz::util::log::message("OpenGL context detected, initialising tz::graphics...");
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			if(print_progress)
				tz::util::log::error("Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Topaz Error", (std::string("Topaz graphics module (tz::graphics) failed to initialise.\nError message:\n ") + std::string(SDL_GetError())).c_str(), NULL);
        }
		else if(print_progress)
			tz::util::log::message("Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).");
		if(TTF_Init() == 0 && print_progress)
			tz::util::log::message("Successfully initialised SDL2_ttf.");
		else if(print_progress)
			tz::util::log::error("Initialisation of SDL2_ttf failed.");
		tz::graphics::initialised = true;
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
	}
	
	inline void terminate(bool print_progress)
	{
		TTF_Quit();
		if(print_progress)
		{
			tz::util::log::message("Terminated SDL2_ttf");
			tz::util::log::message("Terminated tz::graphics via GLEW (OpenGL).");
		}
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
}