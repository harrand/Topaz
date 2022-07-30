#if TZ_OGL
#include "tz/core/containers/basic_list.hpp"
#include "tz/core/report.hpp"
#include "tz/core/profiling/zone.hpp"
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	tz::BasicList<OGLStringView> get_supported_ogl_extensions();

	bool initialised = false;

	void debug_callback([[maybe_unused]] GLenum source, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, GLenum severity, [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* userdata)
	{
		if(type == GL_DEBUG_TYPE_ERROR && (severity & (GL_DEBUG_SEVERITY_HIGH | GL_DEBUG_SEVERITY_MEDIUM)))
		{
			tz_error("OpenGL Error: %s", message);
		}
	}

	void initialise()
	{
		TZ_PROFZONE("OpenGL Backend - Backend Initialise", TZ_PROFCOL_RED);
		tz_assert(!initialised, "Already initialised OpenGL but trying to do it again. Please submit a bug report.");
		int res = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
		#if TZ_PROFILE
			TracyGpuContext;
		#endif // TZ_PROFILE
		tz_assert(res != 0, "GLAD failed to load");
		#if TZ_DEBUG
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(debug_callback, nullptr);
		#endif
		if(!supports_bindless_textures())
		{
			tz_warning_report("The OpenGL backend implicitly uses bindless textures under-the-hood, and initialisation has just detected that the extension `GL_ARB_bindless_texture` is not available. The application will most certainly crash if you try to use OGL textures.");
		}
		tz_report("OpenGL v%u.%u Initialised", ogl_version.major, ogl_version.minor);
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

	tz::BasicList<OGLStringView> get_supported_ogl_extensions()
	{
		GLint extension_count;
		glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
		tz::BasicList<OGLStringView> exts;
		exts.resize(extension_count);
		for(std::size_t i = 0; std::cmp_less(i, extension_count); i++)
		{
			exts[i] = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
		}
		return exts;
	}

	bool supports_bindless_textures()
	{
		return get_supported_ogl_extensions().contains("GL_ARB_bindless_texture");
	}
}

#endif // TZ_OGL
