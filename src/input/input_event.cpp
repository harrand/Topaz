//
// Created by Harrand on 13/12/2019.
//
#include "input/input_event.hpp"
#include "GLFW/glfw3.h"

namespace tz::input
{

	const char* KeyPressEvent::get_key_name() const
	{
		return glfwGetKeyName(this->key, this->scancode);
	}

	char32_t CharPressEvent::get_char() const
	{
		return this->codepoint;
	}
}