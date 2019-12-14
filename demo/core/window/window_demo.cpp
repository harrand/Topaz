//
// Created by Harrand on 13/12/2019.
//

#include "input/input_listener.hpp"
#include "core/core.hpp"
#include "window_demo.hpp"
#include "core/debug/assert.hpp"
#include "core/debug/print.hpp"
#include "GLFW/glfw3.h"

WindowDemo::WindowDemo(): window(nullptr)
{
	tz::core::initialise("WindowDemo");
	this->window = &tz::core::get().window();
	this->window->register_this();
	this->window->emplace_custom_key_listener(
			[this](tz::input::KeyPressEvent kpe)
			{
				if(kpe.action == GLFW_PRESS || kpe.action == GLFW_REPEAT)
				{
					if (kpe.key == GLFW_KEY_ESCAPE)
					{
						this->window->request_close();
						tz::ext::glfw::simulate_typing("off i go then!");
					}
				}
			}
	);
	this->window->emplace_custom_type_listener(
			[](tz::input::CharPressEvent cpe)
			{
				tz::debug_printf("%c", cpe.codepoint);
			}
	);
}

WindowDemo::~WindowDemo()
{
	tz::core::terminate();
}

bool WindowDemo::playing() const
{
	topaz_assert(this->window != nullptr, "WindowDemo::playing(): Underlying window handle is not set! Did you forget to tz::initialise()?");
	return !this->window->is_close_requested();
}

void WindowDemo::update() const
{
	topaz_assert(this->window != nullptr, "");
	this->window->update();
}

int main(int argc, char** argv)
{
	WindowDemo demo;
	while(demo.playing())
	{
		demo.update();
		tz::core::update();
	}
	return 0;
}