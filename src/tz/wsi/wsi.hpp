#ifndef TZ_WSI_TGE_HPP
#define TZ_WSI_TGE_HPP
#include "tz/core/data/handle.hpp"
#include "tz/lua/api.hpp"

namespace tz::wsi
{
	void initialise();
	void terminate();
	void update();

	void lua_initialise(tz::lua::state& state);

	/**
	 * @ingroup tz_cpp
	 * @defgroup tz_wsi Window System Integration
	 * Documentation for functionality related to windowing, peripherals and input.
	 */

	/**
	 * @ingroup tz_wsi
	 * @defgroup tz_wsi_window Window
	 * Create, destroy or customise an application window.
	 */

	/**
	 * @ingroup tz_wsi
	 * @defgroup tz_wsi_monitor Monitors
	 * Retrieve information about hardware monitors currently connected to the machine.
	 */

	/**
	 * @ingroup tz_wsi
	 * @defgroup tz_wsi_mouse Mouse Input
	 * Utilities, structures and helpers for mouse input.
	 */

	/**
	 * @ingroup tz_wsi
	 * @defgroup tz_wsi_keyboard Keyboard Input
	 * Utilities, structures and helpers for keyboard input.
	 */
}

#endif // TZ_WSI_TGE_HPP
