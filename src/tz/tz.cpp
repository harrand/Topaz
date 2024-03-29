#include "tz/tz.hpp"
#include "tz/core/tz_core.hpp"
#include "tz/wsi/wsi.hpp"
#include "tz/tz.hpp"
#include "tz/core/job/job.hpp"
#include "tz/core/debug.hpp"
#include "tz/core/profile.hpp"
#include "tz/dbgui/dbgui.hpp"
#include "tz/lua/slab.hpp"

#if TZ_VULKAN
#include "tz/gl/impl/vulkan/detail/tz_vulkan.hpp"
#elif TZ_OGL
#include "tz/gl/impl/opengl/detail/tz_opengl.hpp"
#endif

#include <cstdio>

namespace tz
{
	tz::wsi::window_handle wnd = tz::nullhand;
	bool initialised = false;
	initialise_info init_info = {};

	void initialise(initialise_info init)
	{
		TZ_PROFZONE("Topaz Initialise", 0xFFFFAA00);
		tz::core::initialise();
		tz::report("%s v%u.%u.%u (%s)", init.name, init.version.major, init.version.minor, init.version.patch, tz::info().to_string().c_str());
		tz::wsi::initialise();
		[[maybe_unused]] tz::game_info game_info{.name = init.name, .version = init.version, .engine = tz::info()};
		// Ensure we're not already initialised before doing anything.
		tz::assert(wnd == tz::nullhand && !initialised, "tz::initialise(): Already initialised (wnd = %p, init = %d)", wnd, initialised);

		// After that, create the window.
		{
			std::string window_title = init.name;
			#if TZ_DEBUG
				window_title = game_info.to_string();
			#endif // TZ_DEBUG

			tz::wsi::window_flag::flag_bit flags = 0;
			if(TZ_OGL)
			{
				flags |= tz::wsi::window_flag::opengl;
			}
			if(init.flags.contains(tz::application_flag::window_hidden))
			{
				flags |= tz::wsi::window_flag::invisible;
			}
			if(init.flags.contains(tz::application_flag::window_transparent))
			{
				flags |= tz::wsi::window_flag::transparent;
			}
			wnd = tz::wsi::create_window
			({
				.title = window_title.c_str(),
				.dimensions = init.dimensions,
				.window_flags = flags
			});
		}
		// Finally, initialise render backends.
		if(!init.flags.contains(tz::application_flag::no_graphics))
		{
			#if TZ_VULKAN
				tz::gl::vk2::initialise(game_info);
			#elif TZ_OGL
				tz::gl::ogl2::initialise();
			#endif
		}
		initialised = true;
		init_info = init;

		tz::dbgui::initialise
		({
	   		.game_info = game_info,
	   		.graphics_enabled = !init.flags.contains(tz::application_flag::no_graphics),
	   		.dbgui_enabled = !init.flags.contains(tz::application_flag::no_dbgui)
		});
	}

	void terminate()
	{
		TZ_PROFZONE("Topaz Terminate", 0xFFFFAA00);
		tz::assert(wnd != tz::nullhand && initialised, "tz::terminate(): Not initialised");
		tz::dbgui::terminate();
		if(!init_info.flags.contains(tz::application_flag::no_graphics))
		{
		tz::gl::destroy_device();
		#if TZ_VULKAN
			tz::gl::vk2::terminate();
		#elif TZ_OGL
			tz::gl::ogl2::terminate();
		#endif
		}

		tz::wsi::destroy_window(wnd);
		wnd = tz::nullhand;
		tz::wsi::terminate();
		tz::core::terminate();
		initialised = false;
	}

	void begin_frame()
	{
		TZ_PROFZONE("Begin Frame", 0xFFFFAA00);
		TZ_FRAME_BEGIN;
		tz::dbgui::begin_frame();
		tz::job_system().new_frame();
		if(!init_info.flags.contains(tz::application_flag::no_graphics))
		{
			tz::gl::get_device().begin_frame();
		}
	}

	void end_frame()
	{
		TZ_PROFZONE("End Frame", 0xFFFFAA00);
		tz::dbgui::end_frame();
		tz::wsi::update();
		tz::lua::process_all_slabs();
		if(!init_info.flags.contains(tz::application_flag::no_graphics))
		{
			tz::gl::get_device().end_frame();
		}
		TZ_FRAME_END;
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
