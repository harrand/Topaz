#if TZ_OGL
#include "core/report.hpp"
#include "gl/impl/backend/ogl2/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	bool initialised = false;

	void debug_callback([[maybe_unused]] GLenum source, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, GLenum severity, [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* userdata)
	{
		if(type == GL_DEBUG_TYPE_ERROR && (severity & (GL_DEBUG_SEVERITY_HIGH | GL_DEBUG_SEVERITY_MEDIUM)))
		{
			tz_error("OpenGL Error: %s", message);
		}
	}

	void initialise([[maybe_unused]] tz::GameInfo game_info, tz::ApplicationType app_type)
	{
		tz_assert(!initialised, "Already initialised OpenGL but trying to do it again. Please submit a bug report.");
		tz_assert(app_type != tz::ApplicationType::Headless, "Headless OpenGL applications are not yet supported.");
		int res = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		tz_assert(res != 0, "GLAD failed to load");
		#if TZ_DEBUG
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(debug_callback, nullptr);
		#endif
		tz_report("OpenGL v%u.%u Initialised (%s)", ogl_version.major, ogl_version.minor, app_type == tz::ApplicationType::Headless ? "Headless" : "Windowed");
		initialised = true;
	}

	void terminate()
	{
		tz_assert(initialised, "Not initialised when trying to terminate OpenGL. Please submit a bug report.");
		initialised = false;
		tz_report("OpenGL v%u.%u Terminated", ogl_version.major, ogl_version.minor);
	}

	bool is_initialised()
	{
		return initialised;
	}
}

#endif // TZ_OGL
