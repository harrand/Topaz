#include "tz/core/tz.hpp"
#include "tz/core/assert.hpp"
#include "tz/core/report.hpp"
#include "tz/core/peripherals/monitor.hpp"
#include "tz/core/profiling/zone.hpp"

#if TZ_VULKAN
#include "tz/gl/impl/backend/vk2/tz_vulkan.hpp"
#elif TZ_OGL
#include "tz/gl/impl/backend/ogl2/tz_opengl.hpp"
#endif

#include <cstdio>

namespace tz
{
	tz::Window* wnd = nullptr;
	bool initialised = false;
	InitialiseInfo init_info = {};

	void initialise(InitialiseInfo init)
	{
		TZ_PROFZONE("Topaz Initialise", TZ_PROFCOL_BLUE);
		tz_report("%s v%u.%u.%u (%s)", init.name, init.version.major, init.version.minor, init.version.patch, tz::info().to_string().c_str());
		[[maybe_unused]] tz::GameInfo game_info{.name = init.name, .version = init.version, .engine = tz::info()};
		// Ensure we're not already initialised before doing anything.
		tz_assert(wnd == nullptr && !initialised, "tz::initialise(): Already initialised (wnd = %p, init = %d)", wnd, initialised);

		// Firstly, initialise GLFW.
		{
			TZ_PROFZONE("GLFW Initialise", TZ_PROFCOL_BLUE);
			[[maybe_unused]] int glfw_ret = glfwInit();
			tz_assert(glfw_ret == GLFW_TRUE, "GLFW initialisation returned without crashing, but we still failed to initialise. Most likely a platform-specific error has occurred. Does your machine support window creation?");
		}
		// Then, initialise peripherals.
		{
			tz::detail::peripherals::monitor::initialise();
		}
		// After that, create the window.
		{
			std::string window_title = init.name;
			#if TZ_DEBUG
				window_title = game_info.to_string();
			#endif // TZ_DEBUG

			WindowInitArgs wargs
			{
				.width = static_cast<int>(init.dimensions[0]),
				.height = static_cast<int>(init.dimensions[1]),
				.title = window_title.c_str(),
				.flags =
				{
					.resizeable = !init.flags.contains(ApplicationFlag::UnresizeableWindow),
					.invisible = init.flags.contains(ApplicationFlag::HiddenWindow)
				}
			};

			wnd = new tz::Window{wargs};
		}
		// Finally, initialise render backends.
		{
			#if TZ_VULKAN
				tz::gl::vk2::initialise(game_info);
			#elif TZ_OGL
				tz::gl::ogl2::initialise();
			#endif
		}

		initialised = true;
		init_info = init;
	}

	void terminate()
	{
		TZ_PROFZONE("Topaz Terminate", TZ_PROFCOL_BLUE);
		tz_assert(wnd != nullptr && initialised, "tz::terminate(): Not initialised");
		#if TZ_VULKAN
			tz::gl::vk2::terminate();
		#elif TZ_OGL
			tz::gl::ogl2::terminate();
		#endif

		delete wnd;

		tz::detail::peripherals::monitor::terminate();
		{
			TZ_PROFZONE("GLFW Terminate", TZ_PROFCOL_BLUE);
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
