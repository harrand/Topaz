//
// Created by Harrand on 13/12/2019.
//

#include "window.hpp"
#include "core/debug/assert.hpp"
#include "gl/frame.hpp"
#include "GLFW/glfw3.h"
#include "input/input_event.hpp"
#include "algo/container.hpp"
#include <algorithm>

namespace tz
{
	tz::input::KeyListener& IWindow::emplace_custom_key_listener()
	{
		std::unique_ptr<tz::input::KeyListener> custom_listener_ptr = std::make_unique<tz::input::CustomKeyListener<tz::input::detail::NullKeyCallback>>();
		tz::input::KeyListener* ptr = custom_listener_ptr.get();
		this->register_key_listener(std::move(custom_listener_ptr));
		return *ptr;
	}
}