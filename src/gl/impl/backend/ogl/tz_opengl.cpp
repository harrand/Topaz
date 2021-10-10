#if TZ_OGL
#include "gl/impl/backend/ogl/tz_opengl.hpp"
#include "core/assert.hpp"
#include "core/report.hpp"

namespace tz::gl::ogl
{
	OpenGLInstance* inst = nullptr;

	void initialise_headless(tz::GameInfo game_info)
	{
		//tz_error("Headless is not supported for OpenGL. Sorry.");
		tz_assert(inst == nullptr, "Already initialised");
		inst = new OpenGLInstance{game_info};
		tz::Version v = ogl::get_opengl_version();
		tz_report("OpenGL v%u.%u, Initialised (Headless)", v.major, v.minor);
	}

	void initialise(tz::GameInfo game_info)
	{
		tz_assert(inst == nullptr, "Already initialised");
		inst = new OpenGLInstance{game_info};
		tz::Version v = ogl::get_opengl_version();
		tz_report("OpenGL v%u.%u Initialised (Window)", v.major, v.minor);
	}

	void terminate()
	{
		tz_assert(inst != nullptr, "Not initialised");
		delete inst;
		inst = nullptr;
		tz_report("OpenGL Terminated");
	}

	OpenGLInstance& get()
	{
		tz_assert(inst != nullptr, "Never initialised");
		return *inst;
	}

	bool is_initialised()
	{
		return inst != nullptr;
	}
}

#endif // TZ_OGL