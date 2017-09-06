#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "glew.h"
#include "utility.hpp"

namespace tz
{
	namespace graphics
	{
		extern bool has_context;
		extern bool initialised;
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
				tz::util::log::error("Initialisation of GLEW failed.\n\tInitialisation of tz::graphics unsuccessful!");
			else
				tz::util::log::message("Initialisation of GLEW successful.\n\tInitialised tz::graphics via GLEW (OpenGL).");
			tz::graphics::initialised = true;
		}
		inline void terminate()
		{
			tz::util::log::message("Terminated tz::graphics via GLEW (OpenGL).");
		}
	}
}

#endif