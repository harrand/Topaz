#include "tz/core/tz.hpp"
#include "tz/wsi/wsi.hpp"
#include "hdk/hdk.hpp"
#include "hdk/debug.hpp"
#include "hdk/profile.hpp"
#include "tz/dbgui/dbgui.hpp"

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#endif

#include <cstdio>

namespace tz
{
	tz::wsi::window_handle wnd = hdk::nullhand;
	bool initialised = false;
	InitialiseInfo init_info = {};

	void initialise(InitialiseInfo init)
	{
		HDK_PROFZONE("Topaz Initialise", 0xFF0000AA);
		hdk::initialise();
		hdk::report("%s v%u.%u.%u (%s)", init.name, init.version.major, init.version.minor, init.version.patch, tz::info().to_string().c_str());
		tz::wsi::initialise();
		[[maybe_unused]] tz::GameInfo game_info{.name = init.name, .version = init.version, .engine = tz::info()};
		// Ensure we're not already initialised before doing anything.
		hdk::assert(wnd == hdk::nullhand && !initialised, "tz::initialise(): Already initialised (wnd = %p, init = %d)", wnd, initialised);

		// After that, create the window.
		{
			std::string window_title = init.name;
			#if HDK_DEBUG
				window_title = game_info.to_string();
			#endif // HDK_DEBUG

			wnd = tz::wsi::create_window
			({
				.title = window_title.c_str(),
				.dimensions = init.dimensions,
				#if TZ_OGL
				.window_flags = tz::wsi::window_flag::opengl
				#endif
			});
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

		tz::dbgui::initialise({.game_info = game_info});
	}

	void terminate()
	{
		HDK_PROFZONE("Topaz Terminate", 0xFF0000AA);
		hdk::assert(wnd != hdk::nullhand && initialised, "tz::terminate(): Not initialised");
		tz::dbgui::terminate();
		tz::gl::destroy_device();
		#if TZ_VULKAN
			tz::gl::vk2::terminate();
		#elif TZ_OGL
			tz::gl::ogl2::terminate();
		#endif

		tz::wsi::destroy_window(wnd);
		wnd = hdk::nullhand;
		tz::wsi::terminate();
		hdk::terminate();
		initialised = false;
	}

	tz::wsi::window& window()
	{
		return tz::wsi::get_window(wnd);
	}

	bool is_initialised()
	{
		return initialised;
	}
}
