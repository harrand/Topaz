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
			tz::util::log::error("Initialisation of GLEW failed.\n\t_initialisation of tz::graphics unsuccessful!");
		else
			tz::util::log::message("Initialisation of GLEW successful.\n\t_initialised tz::graphics via GLEW (OpenGL).");
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