//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_INPUT_EVENT_HPP
#define TOPAZ_INPUT_EVENT_HPP

namespace tz::input
{
	struct KeyPressEvent
	{
		int key;
		int scancode;
		int action;
		int mods;
	};
	
	struct CharPressEvent
	{
		unsigned int codepoint;
	};

	struct MouseUpdateEvent
	{
		double xpos;
		double ypos;
	};
}

#endif //TOPAZ_INPUT_EVENT_HPP
