#ifndef TOPAZ_DBGUI_DBGUI_HPP
#define TOPAZ_DBGUI_DBGUI_HPP
#include "tz/core/game_info.hpp"
#include "tz/core/types.hpp"
#include "tz/core/callback.hpp"
#include "tz/gl/device.hpp"
#include "tz/core/debug.hpp"
#include "imgui.h"
#include "imgui_memory_editor.h"

namespace tz::dbgui
{
	/**
	 * @ingroup tz_cpp
	 * @defgroup tz_dbgui Debug UI
	 *
	 * On debug builds, debug-ui is always displayed on the main window above anything being rendered. When debug-ui is enabled, this API is useful for allowing easy debug-only user activity (such as tweaking a value without recompiling).
	 *
	 * If you're experienced using ImGui, you're free to `#include "tz/dbgui/dbgui.hpp"` and start using ImGui functions to your hearts content. If you're not comfortable using ImGui, [now's a great time to learn](https://github.com/ocornut/imgui#the-pitch).
	 *
	 * @note Debug-UI functionality (and therefore imgui functions) cannot be used or invoked before @ref tz::initialise.
	 */
	struct init_info
	{
		/// Information about the running application.
		tz::game_info game_info;
		bool graphics_enabled;
		bool dbgui_enabled;
	};

	/**
	 * Initialise dbgui. Debug-ui does not function unless this is done.
	 * 
	 * @note This is automatically invoked by @ref tz::initialise on `TZ_DEBUG` builds. You almost never want to invoke this yourself.
	 *
	 * @note In non-debug builds, this does nothing and thus ImGui functions cannot be called.
	 */
	void initialise(init_info info);
	/**
	 * Terminate dbgui. This must be done at the end of the program, after the debug-ui is ready to be shut down.
	 *
	 * @note This is automatically invoked by @ref tz::terminate on `TZ_DEBUG` builds. You almost never want to invoke this yourself.
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
	 * As dbgui is only available on TZ_DEBUG, calling ImGui functions directly in game-code will yield runtime errors on non-debug builds. ImGui function invocations should instead be stored within a lambda and passed to this function, which will do nothing on non-debug builds, and can do extra debug-only state checks.
	 *
	 * Example:
	 * ```cpp
	 *	tz::dbgui::run([]()
	 *	{
	 *		ImGui::Text("This is some text.");
	 *	}
	 * ```
	 */
	inline void run([[maybe_unused]] tz::action auto action)
	{
		#if TZ_DEBUG
			action();
		#endif
	}

	void add_to_lua_log(std::string msg);

	/// Represents a function taking no parameters and returning void.
	using game_menu_callback_type = tz::callback<>;
	using game_bar_callback_type = game_menu_callback_type;
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
	game_menu_callback_type& game_menu();
	game_bar_callback_type& game_bar();

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
