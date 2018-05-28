namespace tz::graphics
{
	inline void initialise()
	{
		if(!tz::graphics::has_context)
		{
			tz::util::log::warning("Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.");
			return;
		}
		tz::util::log::message("OpenGL context detected, initialising tz::graphics...");
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			tz::util::log::error("Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Topaz Error", (std::string("Topaz graphics module (tz::graphics) failed to initialise.\nError message:\n ") + std::string(SDL_GetError())).c_str(), NULL);
        }
		else
			tz::util::log::message("Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).");
		if(TTF_Init() == 0)
			tz::util::log::message("Successfully initialised SDL2_ttf.");
		else
			tz::util::log::error("Initialisation of SDL2_ttf failed.");
		tz::graphics::initialised = true;
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
	}
	
	inline void terminate()
	{
		TTF_Quit();
		tz::util::log::message("Terminated SDL2_ttf");
		tz::util::log::message("Terminated tz::graphics via GLEW (OpenGL).");
	}
}