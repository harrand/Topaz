#ifndef TOPAZ_CORE_KEYBOARD_STATE_HPP
#define TOPAZ_CORE_KEYBOARD_STATE_HPP
#include "GLFW/glfw3.h"
#include <vector>

namespace tz
{
    enum class KeyAction : int
    {
        Press = GLFW_PRESS,
        Release = GLFW_RELEASE,
        Repeat = GLFW_REPEAT
    };

    class KeyPressedState
    {
    public:
        KeyPressedState() = default;
        void glfw_update_state(int key, int scancode, int action, int mods);
        void clear_state();
        bool is_pressed(int key) const;
    private:
        std::vector<int> pressed_keycodes;
    };
}

#endif // TOPAZ_CORE_KEYBOARD_STATE_HPP