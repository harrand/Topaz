#ifndef TOPAZ_CORE_KEYBOARD_STATE_HPP
#define TOPAZ_CORE_KEYBOARD_STATE_HPP
#if TZ_VULKAN
// "To include the Vulkan header, define GLFW_INCLUDE_VULKAN before including the GLFW header." - GLFW
// We will want to use glfw-vulkan integration functionality later, so we make sure those headers are included properly now, even through strictly keyboard state has nothing to do with vulkan.
#include "vulkan/vulkan.h"
#endif // TZ_VULKAN
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
