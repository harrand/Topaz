#include "core/tz.hpp"
#include "core/assert.hpp"
#include "core/report.hpp"

#if TZ_VULKAN
#include "gl/impl/backend/vk2/tz_vulkan.hpp"
#elif TZ_OGL
#include "gl/impl/backend/ogl/tz_opengl.hpp"
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
			glfwInit();
			tz_assert(wnd == nullptr && !initialised, "tz::initialise(): Already initialised");
			WindowHintList hints;
			if(app_type == ApplicationType::HiddenWindowApplication)
			{
				hints.add(WindowHint{.hint = GLFW_VISIBLE, .value = GLFW_FALSE});
			}

			std::string wnd_title = game_info.to_string();
			wargs.title = wnd_title.c_str();
			wnd = new tz::Window{wargs, hints};
		}
		else
		{
			#if TZ_VULKAN
				wnd = new tz::Window{tz::Window::null()};
			#elif TZ_OGL
				// TODO: Headless implementation for OGL
				tz_report("Headless applications are stubbed out for OpenGL");
				std::exit(0);
			#endif
		}
		
		tz_report("%s Application", app_type == ApplicationType::Headless ? "Headless" : "Windowed");
		initialised = true;
		#if TZ_VULKAN
			tz::gl::vk2::initialise(game_info, app_type);
		#elif TZ_OGL
			if(app_type == ApplicationType::WindowApplication || app_type == ApplicationType::HiddenWindowApplication)
			{
				tz::gl::ogl::initialise(game_info);
			}
			else if(app_type == ApplicationType::Headless)
			{
				tz::gl::ogl::initialise_headless(game_info);
			}
			else
			{
				tz_error("Invalid tz::ApplicationType or support for that type is not yet implemented for this render-api (OpenGL)");
			}
		#endif

		tz_app_type = app_type;
	}

	void terminate()
	{
		#if TZ_VULKAN
			tz::gl::vk2::terminate();
		#elif TZ_OGL
			tz::gl::ogl::terminate();
		#endif

		if(tz_app_type == ApplicationType::WindowApplication)
		{
			tz_assert(wnd != nullptr && initialised, "tz::terminate(): Not initialised");
			delete wnd;
		}

		if(tz_app_type != ApplicationType::Headless)
		{
			glfwTerminate();
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
