#include "core/peripherals/keyboard.hpp"
#include "core/assert.hpp"

namespace tz
{
	void KeyboardState::update(KeyPressInfo info)
	{
		auto iter = std::find_if(this->pressed_keys.begin(), this->pressed_keys.end(),
		[info](const KeyPressInfo& pressed)
		{
			return pressed.key.code == info.key.code;
		});

		switch(info.type)
		{
			case KeyPressType::Press:
				if(iter == this->pressed_keys.end())
				{
					// We need to add this.
					this->pressed_keys.push_back(info);
				}
				else
				{
					// We need to set it as repeat.
					iter->type = KeyPressType::Repeat;
				}
			break;
			case KeyPressType::Release:
				if(iter != this->pressed_keys.end())
				{
					iter = this->pressed_keys.erase(iter);
				}
			break;
			default:
				tz_error("Key  press update should only be a press or release type. Detected another key press type.");
			break;
		}
	}

	bool KeyboardState::is_key_down(KeyCode key) const
	{
		return std::find_if(this->pressed_keys.begin(), this->pressed_keys.end(),
		[key](const KeyPressInfo& info){return info.key.code == key;}) != this->pressed_keys.end();
	}

	bool KeyboardState::is_key_repeating(KeyCode key) const
	{
		auto iter = std::find_if(this->pressed_keys.begin(), this->pressed_keys.end(),
		[key](const KeyPressInfo& info){return info.key.code == key;});
		return iter != this->pressed_keys.end() && iter->type == KeyPressType::Repeat;
	}

	void KeyboardState::debug_print_state() const
	{
		#if TZ_DEBUG
			for(const KeyPressInfo& info : this->pressed_keys)
			{
				std::printf("{%c, %s}", info.key.representation, info.type == KeyPressType::Press ? "Press" : "Repeat");
			}
			std::printf("                                                        \r");
		#endif
	}
}
