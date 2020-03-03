//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_INPUT_EVENT_HPP
#define TOPAZ_INPUT_EVENT_HPP

namespace tz::input
{
	/**
     * \addtogroup tz_input Topaz Input Library (tz::input)
     * A collection of platform-agnostic input functions and classes.
     * @{
     */

	struct KeyPressEvent
	{
		int key;
		int scancode;
		int action;
		int mods;

		const char* get_key_name() const;
	};
	
	struct CharPressEvent
	{
		// Because an unsigned int is 32 bits long on all platforms supported by GLFW, you can treat the code point argument as native endian UTF-32.
		unsigned int codepoint;

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

	/**
	 * @}
	 */
}

#endif //TOPAZ_INPUT_EVENT_HPP
