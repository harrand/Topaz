#if TZ_OGL
#include "tz/core/data/basic_list.hpp"
#include "tz/wsi/window.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"

namespace tz::gl::ogl2
{
	tz::basic_list<OGLStringView> get_supported_ogl_extensions();

	bool initialised = false;

	void debug_callback([[maybe_unused]] GLenum source, [[maybe_unused]] GLenum type, [[maybe_unused]] GLuint id, GLenum severity, [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* userdata)
	{
		if(type == GL_DEBUG_TYPE_ERROR && (severity & (GL_DEBUG_SEVERITY_HIGH | GL_DEBUG_SEVERITY_MEDIUM)))
		{
			tz::error("OpenGL Error: %s", message);
		}
	}

	void initialise()
	{
		TZ_PROFZONE("OpenGL Backend - Backend Initialise", 0xFFAA0000);
		tz::assert(!initialised, "Already initialised OpenGL but trying to do it again. Please submit a bug report.");
		int res = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(tz::wsi::get_opengl_proc_address));
		#if TZ_PROFILE
			TracyGpuContext;
		#endif // TZ_PROFILE
		tz::assert(res != 0, "GLAD failed to load");
		#if TZ_DEBUG
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(debug_callback, nullptr);
		#endif
		if(!supports_bindless_textures())
		{
			tz::report("The OpenGL backend prefers using bindless textures under-the-hood. Unfortunately, the bindless textures extension `GL_ARB_bindltess_texture` is unavailable on this implementation. Renderers will fallback to old-style OpenGL uniforms for image resources, although this behaviour is unreliable and could be removed at any point.");
		}
		tz::report("OpenGL v%u.%u %sInitialised", ogl_version.major, ogl_version.minor, supports_bindless_textures() ? "" : "(Bindful) ");
		initialised = true;
	}

	void terminate()
	{
		tz::assert(initialised, "Not initialised when trying to terminate OpenGL. Please submit a bug report.");
		initialised = false;
		tz::report("OpenGL v%u.%u Terminated", ogl_version.major, ogl_version.minor);
	}

	bool is_initialised()
	{
		return initialised;
	}

	tz::basic_list<OGLStringView> get_supported_ogl_extensions()
	{
		GLint extension_count;
		glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
		tz::basic_list<OGLStringView> exts;
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
