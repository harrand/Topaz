#ifndef TOPAZ_DBGUI_DBGUI_HPP
#define TOPAZ_DBGUI_DBGUI_HPP
#include "tz/core/game_info.hpp"
#include "tz/core/types.hpp"
#include "tz/core/callback.hpp"
#include "tz/gl/device.hpp"
#include "imgui.h"

#undef assert

namespace tz::dbgui
{
	/**
	 * @defgroup tz_dbgui Debug UI (tz::dbgui)
	 * Functionality for debug-only user interface via ImGui.
	 *
	 * On debug builds, debug-ui is always displayed on the main window above anything being rendered. When debug-ui is enabled, this API is useful for allowing easy debug-only user activity (such as tweaking a value without recompiling).
	 *
	 * If you're experienced using ImGui, you're free to `#include "tz/dbgui/dbgui.hpp"` and start using ImGui functions to your hearts content. If you're not comfortable using ImGui, [now's a great time to learn](https://github.com/ocornut/imgui#the-pitch).
	 *
	 * @note Debug-ui functionality (and therefore imgui functions) cannot be used or invoked before @ref tz::initialise.
	 */
	struct InitInfo
	{
		/// Information about the running application.
		tz::GameInfo game_info;
	};

	/**
	 * Initialise dbgui. Debug-ui does not function unless this is done.
	 * 
	 * @note This is automatically invoked by @ref tz::initialise on `HDK_DEBUG` builds. You almost never want to invoke this yourself.
	 *
	 * @note In non-debug builds, this does nothing and thus ImGui functions cannot be called.
	 */
	void initialise(InitInfo info);
	/**
	 * Terminate dbgui. This must be done at the end of the program, after the debug-ui is ready to be shut down.
	 *
	 * @note This is automatically invoked by @ref tz::terminate on `HDK_DEBUG` builds. You almost never want to invoke this yourself.
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
	 * @ingroup tz_dbgui
	 * As dbgui is only available on HDK_DEBUG, calling ImGui functions directly in game-code will yield runtime errors on non-debug builds. ImGui function invocations should instead be stored within a lambda and passed to this function, which will do nothing on non-debug builds, and can do extra debug-only state checks.
	 *
	 * Example:
	 * ```cpp
	 *	tz::dbgui::run([]()
	 *	{
	 *		ImGui::Text("This is some text.");
	 *	}
	 * ```
	 */
	inline void run([[maybe_unused]] tz::Action auto action)
	{
		#if HDK_DEBUG
			action();
		#endif
	}

	/// Represents a function taking no parameters and returning void.
	using GameMenuCallbackType = tz::Callback<>;
	/**
	 * @ingroup tz_dbgui
	 * Retrieve the game menu callback.
	 *
	 * Use this if you want to add menu items to the top imgui menu under your application name.
	 *
	 * Example:
	 * ```cpp
	 *	bool game_menu_enabled = false; 
	 *	tz::dbgui::game_menu().add_callback([&game_menu_enabled]()
	 *	{
	 *		ImGui::MenuItem("Control Panel", nullptr, &game_menu_enabled);
	 *	});
	 *	if(game_menu_enabled){...}
	 * ```
	 */
	GameMenuCallbackType& game_menu();

	/**
	 * @ingroup tz_dbgui
	 * Query as to whether the debug-ui is currently using the keyboard input. You might want to check if this is false before taking key input elsewhere (or the user could input into multiple places at once).
	 *
	 * If dbgui is not available, this always returns `false`.
	 */
	bool claims_keyboard();

	/**
	 * @ingroup tz_dbgui
	 * Query as to whether the debug-ui is currently using the mouse input. You might want to check if this is false before taking mouse input elsewhere (or the user could input into multiple places at once).
	 *
	 * If dbgui is not available, this always returns `false`.
	 */
	bool claims_mouse();
}

#endif // TOPAZ_DBGUI_DBGUI_HPP