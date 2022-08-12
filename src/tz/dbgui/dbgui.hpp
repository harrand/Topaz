#ifndef TOPAZ_DBGUI_DBGUI_HPP
#define TOPAZ_DBGUI_DBGUI_HPP
#include "tz/core/game_info.hpp"
#include "tz/core/types.hpp"
#include "tz/core/callback.hpp"
#include "tz/gl/device.hpp"
#include "imgui.h"

namespace tz::dbgui
{
	struct InitInfo
	{
		tz::GameInfo game_info;
	};

	/**
	 * Initialise dbgui. Debug-ui does not function unless this is done.
	 *
	 * @note In non-debug builds, this does nothing and thus ImGui functions cannot be called.
	 */
	void initialise(InitInfo info);
	/**
	 * Terminate dbgui. This must be done at the end of the program, after the debug-ui is ready to be shut down.
	 *
	 * @pre @ref dbgui::initialise must have been invoked at some point in the past.
	 */
	void terminate();

	/**
	 * Begin the frame. After this, it is valid to call ImGui functions.
	 */
	void begin_frame();
	/**
	 * End the frame. Any requested render work from imgui is carried out here. After this, it is no longer valid to call ImGui functions until the next frame has begun.
	 */
	void end_frame();

	/**
	 * As dbgui is only available on TZ_DEBUG, calling ImGui functions directly in game-code will yield runtime errors on non-debug builds. ImGui function invocations should instead be stored within a lambda and passed to this function, which will do nothing on non-debug builds, and can do extra debug-only state checks.
	 */
	inline void run(tz::Action auto action)
	{
		#if TZ_DEBUG
			action();
		#endif
	}

	using GameMenuCallbackType = tz::Callback<>;
	GameMenuCallbackType& game_menu();
}

#endif // TOPAZ_DBGUI_DBGUI_HPP
