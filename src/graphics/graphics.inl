namespace tz::graphics
{
	inline void initialise(bool print_progress)
	{
		if(!tz::graphics::has_context)
		{
			if(print_progress)
				std::cerr << "Initialisation of tz::graphics aborted: No OpenGL context has been setup yet.\n";
			return;
		}
		if(print_progress)
			std::cout << "OpenGL context detected, initialising tz::graphics...\n";
		GLenum status = glewInit();
		if(status != GLEW_OK)
		{
			if(print_progress)
				std::cerr << "Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!\n";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Topaz Error", (std::string("Topaz graphics module (tz::graphics) failed to initialise.\nError message:\n ") + std::string(SDL_GetError())).c_str(), NULL);
        }
		else if(print_progress)
			std::cout << "Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).\n";
		if(TTF_Init() == 0 && print_progress)
			std::cout << "Successfully initialised SDL2_ttf.\n";
		else if(print_progress)
			std::cout << "Initialisation of SDL2_ttf failed.\n";
		tz::graphics::initialised = true;
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_MULTISAMPLE);
	}
	
	inline void terminate(bool print_progress)
	{
		TTF_Quit();
		if(print_progress)
		{
			std::cout << "Terminated SDL2_ttf\n";
			std::cout << "Terminated tz::graphics via GLEW (OpenGL).\n";
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

	inline void enable_wireframe_render(bool wireframes)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframes ? GL_LINE : GL_FILL);
	}
}