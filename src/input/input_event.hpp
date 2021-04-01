//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_INPUT_EVENT_HPP
#define TOPAZ_INPUT_EVENT_HPP
#include "core/types.hpp"

namespace tz::input
{
	/**
	 * \addtogroup tz_input Topaz Input Library (tz::input)
	 * A collection of platform-agnostic input functions and classes.
	 * @{
	 */

	struct KeyPressEvent
	{
		enum class Action : int
		{
			Pressed,
			Released,
			Repeat
		};

		using Key = int;
		using ScanCode = int;
		using Mods = int;
		Key key;
		ScanCode scancode;
		Action action;
		Mods mods;

		const char* get_key_name() const;
	};
	
	struct CharPressEvent
	{
		using CodePoint = unsigned int;
		// Because an unsigned int is 32 bits long on all platforms supported by GLFW, you can treat the code point argument as native endian UTF-32.
		CodePoint codepoint;

		char32_t get_char() const;		
	};

	struct MouseUpdateEvent
	{
		double xpos;
		double ypos;
	};

	struct MouseClickEvent
	{
		int button;
		int action;
		int mods;
	};

	template<typename F>
	concept KeyPressCallback = tz::Action<F, KeyPressEvent>;

	template<typename F>
	concept CharPressCallback = tz::Action<F, CharPressEvent>;

	template<typename F>
	concept MouseUpdateCallback = tz::Action<F, MouseUpdateEvent>;

	template<typename F>
	concept MouseClickCallback = tz::Action<F, MouseClickEvent>;

	/**
	 * @}
	 */
}

#endif //TOPAZ_INPUT_EVENT_HPP
