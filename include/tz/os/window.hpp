#ifndef TOPAZ_OS_WINDOW_HPP
#define TOPAZ_OS_WINDOW_HPP
#include "tz/core/handle.hpp"
#include "tz/core/error.hpp"
#include <string>

namespace tz::os
{
	/**
	 * @ingroup tz_os
	 * @defgroup tz_os_window Window System
	 * @brief Creating, modifying and destroying OS windows.
	 **/

	/**
	 * @ingroup tz_os_window
	 * @brief Optional behaviours/attributes when opening windows.
	 **/
	enum window_flags
	{
		none = 0x00,
		/// Ignore the x and y coordinates provided within @ref window_info and position the window approximately in the middle of the screen instead.
		centered_window = 0b0001,
		/// Ignore the width and height provided within @ref window_info and set set the window as maximised instead.
		maximised = 0b0010,
		/// When a pixel of the window's framebuffer is never drawn to, instead of being a clamped colour it is instead fully transparent. Your mileage may vary, depending on your platform.
		transparent = 0b0100,
		/// When the window is opened, it is initially invisible and you must invoke @ref window_set_visible to see it.
		invisible = 0b1000,
	};

	constexpr window_flags operator|(window_flags lhs, window_flags rhs)
	{
		return static_cast<window_flags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	constexpr bool operator&(window_flags lhs, window_flags& rhs)
	{
		return static_cast<int>(lhs) & static_cast<int>(rhs);
	}

	/**
	 * @ingroup tz_os_window
	 * @brief Basic customisation of a newly-opened window.
	 **/
	struct window_info
	{
		/// Title of the window.
		std::string title = "Untitled";
		/// x-position of the window, in pixels.
		unsigned int x = 0;
		/// y-position of the window, in pixels.
		unsigned int y = 0;
		/// Width of the window body, in pixels.
		unsigned int width = 800;
		/// Height of the window body, in pixels.
		unsigned int height = 600;
		/// Some optional extra flags. See @ref window_flags.
		window_flags flags = window_flags::none;
	};

	using window_handle = tz::handle<window_info>;

	/**
	 * @ingroup tz_os_window
	 * @brief Open a new window.
	 * @return - @ref error_code::precondition_failure if you have not yet initialised the engine via @ref tz::initialise.
	 * @return - @ref error_code::unknown_error if window creation fails for some other reason.
	 * @return - @ref error_code::partial_success if you opted into @ref window_flags::transparent but your platform/machine does not support it.
	 *
	 * Only one window can be open at a time - If you have called this function before but have not since closed the previous window, it will automatically be closed.
	 **/
	tz::error_code open_window(window_info winfo);
	/**
	 * @ingroup tz_os_window
	 * @brief Close a previously-opened window.
	 * @return - @ref error_code::precondition_failure if you have not yet initialised the engine via @ref tz::initialise.
	 * @return - @ref error_code::precondition_failure if you have not previously opened a window via @ref open_window.
	 * @return - @ref error_code::unknown_error if window destruction fails for some other reason.
	 *
	 * Only one window can be open at a time - If you have called this function before but have not since closed the previous window, it will automatically be closed.
	 **/
	tz::error_code close_window();

	/**
	 * @ingroup tz_os_window
	 * @brief Query as to whether a window has been opened that has not yet been closed.
	 * @return True if an open window is available, false otherwise.
	 **/
	bool window_is_open();
	/**
	 * @ingroup tz_os_window
	 * @brief Poll updates for a window, advancing input events etc.
	 *
	 * If an open window does not exist, this function safely does nothing.
	 **/
	void window_update();	

	/**
	 * @ingroup tz_os_window
	 * @brief Retrieve a opaque handle corresponding to the underlying window native.
	 *
	 * - If you have not previously opened a window, i.e @ref window_is_open() returns false - then this will return @ref nullhand.
	 * - If you are on windows, you can convert this to a `HWND` via `(HWND)(uintptr_t)handle.peek()` 
	 * - TODO: what do on linux?
	 **/
	window_handle get_window_handle();

	/**
	 * @ingroup tz_os_window
	 * @brief Retrieve the width of the window, in pixels.
	 * @note If a window is not open, nothing happens.
	 *
	 * @See @ref window_set_dimensions to programatically resize the window.
	 */
	unsigned int window_get_width();
	/**
	 * @ingroup tz_os_window
	 * @brief Retrieve the height of the window, in pixels.
	 * @note If a window is not open, nothing happens.
	 *
	 * @See @ref window_set_dimensions to programatically resize the window.
	 */
	unsigned int window_get_height();
	/**
	 * @ingroup tz_os_window
	 * @brief Set a new width and height for the open window.
	 * @note If a window is not open, nothing happens.
	 */
	void window_set_dimensions(unsigned int width, unsigned int height);
	/**
	 * @ingroup tz_os_window
	 * @brief Retrieve the title of the window.
	 * @note If a window is not open, an empty string is returned.
	 */
	std::string window_get_title();
	/**
	 * @ingroup tz_os_window
	 * @brief Set a new title for the open window.
	 * @note If a window is not open, nothing happens.
	 */
	void window_set_title(std::string_view title);
	/**
	 * @ingroup tz_os_window
	 * @brief Maximise the window, causing it to cover the whole screen.
	 *
	 * @note If a window is not open, nothing happens.
	 */
	void window_maximise();
	/**
	 * @ingroup tz_os_window
	 * @brief Minimise the window, causing it to no longer be visible until maximised/shown.
	 *
	 * @note If a window is not open, nothing happens.
	 */
	void window_minimise();
	/**
	 * @ingroup tz_os_window
	 * @brief Show the window as normal, even if it is currently maximised/minimised/fullscreen.
	 *
	 * @note If a window is not open, nothing happens.
	 */
	void window_show();
	/**
	 * @ingroup tz_os_window
	 * @brief Display the window in proper fullscreen.
	 *
	 * @note If a window is not open, nothing happens.
	 */
	void window_fullscreen();
}

#endif // TOPAZ_OS_WINDOW_HPP