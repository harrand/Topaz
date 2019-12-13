//
// Created by Harrand on 13/12/2019.
//

#include "core/core.hpp"
#include "window_demo.hpp"
#include "core/debug/assert.hpp"
// TODO: Remove
#include "core/debug/print.hpp"

WindowDemo::WindowDemo(): window(nullptr)
{
	tz::core::initialise("WindowDemo");
	this->window = &tz::core::get().window();
	this->window->set_size(500, 400);
	this->window->set_title("Well met!");
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
		//tz::debug_printf("hahahah im having fun");
	}
	return 0;
}