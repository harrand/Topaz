#include "core/keyboard_state.hpp"
#include <algorithm>

namespace tz
{
    void KeyPressedState::glfw_update_state(int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
    {
        auto a = static_cast<KeyAction>(action);
        switch(a)
        {
            case KeyAction::Press:
                if(!this->is_pressed(key))
                {
                    this->pressed_keycodes.push_back(key);
                }
            break;
            case KeyAction::Release:
                if(this->is_pressed(key))
                {
                    this->pressed_keycodes.erase(std::find(this->pressed_keycodes.begin(), this->pressed_keycodes.end(), key), this->pressed_keycodes.end());
                }
            break;
            default:
            break;
        }
    }

    void KeyPressedState::clear_state()
    {
        this->pressed_keycodes.clear();
    }

    bool KeyPressedState::is_pressed(int key) const
    {
        return std::find(this->pressed_keycodes.begin(), this->pressed_keycodes.end(), key) != this->pressed_keycodes.end();
    }
}