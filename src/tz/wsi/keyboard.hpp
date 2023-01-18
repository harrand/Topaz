#ifndef TZ_WSI_KEYBOARD_HPP
#define TZ_WSI_KEYBOARD_HPP
#include "tz/wsi/api/keyboard.hpp"
#include <string>

namespace tz::wsi
{
	/**
	 * @ingroup tz_wsi_keyboard
	 * Retrieve a human-readable name for a key.
	 */
	std::string get_key_name(tz::wsi::key key);

	/**
	 * @ingroup tz_wsi_keyboard
	 * Retrieve a string representing the characters typed, after modifiers.
	 * @param key Key that has just been pressed.
	 * @param state State of the keyboard when the key was pressed.
	 */
	std::string get_chars_typed(tz::wsi::key key, const keyboard_state& state);
	/**
	 * @ingroup tz_wsi_keyboard
	 * Query as to whether a specific key is currently pressed for a keyboard state.
	 * @param kb Keyboard state to query.
	 * @param key Which key are you asking to see is pressed?
	 * @return True if `key` is pressed according to the passed keyboard state, false if not.
	 */
	bool is_key_down(const keyboard_state& kb, tz::wsi::key key);
}

#endif // TZ_WSI_KEYBOARD_HPP
