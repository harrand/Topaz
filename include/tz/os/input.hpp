#ifndef TOPAZ_OS_INPUT_HPP
#define TOPAZ_OS_INPUT_HPP
#include "tz/core/error.hpp"
#include <utility>

namespace tz::os
{
	/**
	 * @ingroup tz_os
	 * @defgroup tz_os_input Input System
	 * @brief Retrieve keyboard/mouse input.
	 **/

	/**
	 * @ingroup tz_os_input
	 * Represents a key on your keyboard.
	 **/
	enum class key
	{
		backspace,
		tab,
		enter,

		left_shift,
		left_control,
		left_alt,

		pause,
		caps_lock,
		escape,

		spacebar,
		page_down,
		page_up,

		end,
		home,

		left_arrow,
		up_arrow,
		right_arrow,
		down_arrow,

		select,
		print,
		printsc,
		ins,
		del,
		n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,
		a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,

		left_winkey,
		right_winkey,
		apps,
		sleep,

		np0,np1,np2,np3,np4,np5,np6,np7,np8,np9,
		npmul,npadd,npsep,npsub,npdec,npdiv,
		f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11,f12,f13,f14,f15,f16,f17,f18,f19,f20,f21,f22,f23,f24,
		num_lock,
		scroll_lock,

		right_shift,
		right_control,
		right_alt,
		semicol,
		period,
		plus,
		minus,

		obrack,
		cbrack,
		forward_slash,
		back_slash,
		apostrophe,
		grave,
		_count
	};

	using char_type_callback = void(*)(char);
	/**
	 * @ingroup tz_os_input
	 * @brief Install a callback which will be invoked whenever the user types a character on the keyboard.
	 * @param callback Pointer to a function which will be called whenever a character is typed. If you pass nullptr, the callback will safely be ignored.
	 * @return - @ref error_code::precondition_failure if a window has not yet been opened.
	 **/
	tz::error_code install_char_typed_callback(char_type_callback callback);
	/**
	 * @ingroup tz_os_input
	 * @brief Query as to whether a specific key is pressed right now.
	 * @param k Keyboard key to query
	 * @return True if the given key is currently pressed, otherwise false.
	 */
	bool is_key_pressed(key k);
	constexpr std::pair<unsigned int, unsigned int> invalid_mouse_position{-1u, -1u};
	/**
	 * @ingroup tz_os_input
	 * @brief Retrieve the mouse cursor's current position, in pixels, relative to the top-left of the window.
	 *
	 * If for whatever reason the cursor pos cannot be retrieved, {-1, -1} is returned. Some reasons could include:
	 * - You haven't opened a window via @ref open_window.
	 * - The mouse currently lies outside of the window.
	 */
	std::pair<unsigned int, unsigned int> get_mouse_position();

	enum class mouse_button
	{
		left,
		right,
		middle,
		_count
	};

	bool is_mouse_clicked(mouse_button b);
	std::pair<unsigned int, unsigned int> get_mouse_click_position(mouse_button b);
}

#endif // TOPAZ_OS_INPUT_HPP