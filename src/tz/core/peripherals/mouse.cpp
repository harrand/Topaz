#include "tz/core/peripherals/mouse.hpp"
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

	void MouseButtonState::update_scroll(tz::Vec2 offset)
	{
		this->mouse_wheel_offset += offset;
	}

	bool MouseButtonState::is_mouse_button_down(MouseButton button) const
	{
		return std::find_if(this->pressed_buttons.begin(), this->pressed_buttons.end(),
		[button](const MouseButtonPressInfo& info)
		{
			return info.button.button == button;
		}) != this->pressed_buttons.end();
	}

	std::span<const MouseButtonPressInfo> MouseButtonState::get_pressed_buttons() const
	{
		return this->pressed_buttons;
	}

	tz::Vec2 MouseButtonState::get_scroll_offset() const
	{
		return this->mouse_wheel_offset;
	}

	tz::Vec2 MouseButtonState::get_mouse_press_position(MouseButton button) const
	{
		tz_assert(this->is_mouse_button_down(button), "Cannot retrieve mouse press position when the button is not pressed. Please submit a bug report.");
		auto iter = std::find_if(this->pressed_buttons.begin(), this->pressed_buttons.end(),
		[button](const MouseButtonPressInfo& info)
		{
			return info.button.button == button;
		});
		if(iter == this->pressed_buttons.end())
		{
			return {};
		}
		return iter->press_position;
	}

	void MouseButtonState::debug_print_state() const
	{
		#if TZ_DEBUG
			for(const MouseButtonPressInfo& info : this->pressed_buttons)
			{
				std::printf("{%s (%g,%g)}", info.button.button_name, info.press_position[0], info.press_position[1]);
			}
			std::printf("                                                        \r");
		#endif
	}

	void MousePositionState::update(tz::Vec2 position)
	{
		this->info.position = position;
	}

	tz::Vec2 MousePositionState::get_mouse_position() const
	{
		return this->info.position;
	}

	void MousePositionState::debug_print_state() const
	{
		#if TZ_DEBUG
			std::printf("{%g, %g}", this->info.position[0], this->info.position[1]);
			std::printf("                    \r");
		#endif
	}
}
