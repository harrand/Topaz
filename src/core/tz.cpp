#include "core/tz.hpp"
#include "core/assert.hpp"
#include "core/report.hpp"
#include "core/peripherals/monitor.hpp"
#include "core/profiling/zone.hpp"

#if TZ_VULKAN
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#elif TZ_OGL
#include "gl/impl/backend/ogl2/tz_opengl.hpp"
#endif

#include <cstdio>

namespace tz
{
	tz::Window* wnd = nullptr;
	bool initialised = false;
	ApplicationType tz_app_type = ApplicationType::WindowApplication;

	void initialise(GameInfo game_info, ApplicationType app_type, WindowInitArgs wargs)
	{   
		if(app_type == ApplicationType::WindowApplication || app_type == ApplicationType::HiddenWindowApplication)
		{
			{
				TZ_PROFZONE("GLFW Initialise", TZ_PROFCOL_BLUE);
				[[maybe_unused]] int glfw_ret = glfwInit();
				tz_assert(glfw_ret == GLFW_TRUE, "GLFW initialisation returned without crashing, but we still failed to initialise. Most likely a platform-specific error has occurred. Does your machine support window creation?");
			}
			tz::detail::peripherals::monitor::initialise();
			tz_assert(wnd == nullptr && !initialised, "tz::initialise(): Already initialised");
			WindowHintList hints;
			if(app_type == ApplicationType::HiddenWindowApplication)
			{
				hints.add(WindowHint{.hint = GLFW_VISIBLE, .value = GLFW_FALSE});
			}

			std::string wnd_title = game_info.to_string();
			#if !TZ_DEBUG
				wnd_title = game_info.name;
			#endif
			wargs.title = wnd_title.c_str();
			wnd = new tz::Window{wargs, hints};
		}
		else
		{
			#if TZ_VULKAN
				wnd = new tz::Window{tz::Window::null()};
			#elif TZ_OGL
				// TODO: Headless implementation for OGL
				tz_error("Headless applications are stubbed out for OpenGL");
				std::exit(-1);
			#endif
		}
		initialised = true;
		#if TZ_VULKAN
			tz::gl::vk2::initialise(game_info, app_type);
		#elif TZ_OGL
			tz::gl::ogl2::initialise(game_info, app_type);
		#endif

		tz_app_type = app_type;
	}

	void initialise(InitialiseInfo init)
	{
		TZ_PROFZONE("Topaz Initialise", TZ_PROFCOL_BLUE);
		tz_report("%s v%u.%u.%u (%s)", init.name, init.version.major, init.version.minor, init.version.patch, tz::info().to_string().c_str());
		initialise({.name = init.name, .version = init.version, .engine = tz::info()}, init.app_type, init.window);
	}

	void terminate()
	{
		TZ_PROFZONE("Topaz Terminate", TZ_PROFCOL_BLUE);
		#if TZ_VULKAN
			tz::gl::vk2::terminate();
		#elif TZ_OGL
			tz::gl::ogl2::terminate();
		#endif

		if(tz_app_type == ApplicationType::WindowApplication)
		{
			tz_assert(wnd != nullptr && initialised, "tz::terminate(): Not initialised");
			delete wnd;
		}

		if(tz_app_type != ApplicationType::Headless)
		{
			tz::detail::peripherals::monitor::terminate();
			{
				TZ_PROFZONE("GLFW Terminate", TZ_PROFCOL_BLUE);
				glfwTerminate();
			}
		}
		initialised = false;
	}

	Window& window()
	{
		tz_assert(wnd != nullptr, "tz::window(): Not initialised");
		return *wnd;
	}

	bool is_initialised()
	{
		return initialised;
	}
}
