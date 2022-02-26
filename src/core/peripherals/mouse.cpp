#include "core/peripherals/mouse.hpp"
#include <cstdio>

namespace tz
{
	void MouseButtonState::update(MouseButtonPressInfo button, bool pressed)
	{
		auto iter = std::find_if(this->pressed_buttons.begin(), this->pressed_buttons.end(),
		[button](const MouseButtonPressInfo& info)
		{
			return info.button.button == button.button.button;
		});

		if(pressed)
		{
			if(iter == this->pressed_buttons.end())
			{
				this->pressed_buttons.push_back(button);
			}
		}
		else
		{
			if(iter != this->pressed_buttons.end())
			{
				iter = this->pressed_buttons.erase(iter);
			}
		}
	}

	bool MouseButtonState::is_mouse_button_down(MouseButtonInfo button) const
	{
		return std::find_if(this->pressed_buttons.begin(), this->pressed_buttons.end(),
		[button](const MouseButtonPressInfo& info)
		{
			return info.button.button == button.button;
		}) != this->pressed_buttons.end();
	}

	void MouseButtonState::debug_print_state() const
	{
		#if TZ_DEBUG
			for(const MouseButtonPressInfo& info : this->pressed_buttons)
			{
				std::printf("{%s (%u,%u)}", info.button.button_name, info.press_position[0], info.press_position[1]);
			}
			std::printf("                                                        \r");
		#endif
	}

	void MousePositionState::update(tz::Vec2ui position)
	{
		this->info.position = position;
	}

	tz::Vec2ui MousePositionState::get_mouse_position() const
	{
		return this->info.position;
	}

	void MousePositionState::debug_print_state() const
	{
		#if TZ_DEBUG
			std::printf("{%u, %u}", this->info.position[0], this->info.position[1]);
			std::printf("                    \r");
		#endif
	}
}
