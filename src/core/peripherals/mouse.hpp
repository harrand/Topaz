#ifndef TOPAZ_CORE_PERIPHERALS_MOUSE_HPP
#define TOPAZ_CORE_PERIPHERALS_MOUSE_HPP
#include <array>
#include <vector>

namespace tz
{
	/**
	 * @ingroup tz_core_peripherals
	 * @defgroup tz_core_peripherals_mouse Mouse
	 * Documentation for mouse and mouse input.
	 */

	/**
	 * @ingroup tz_core_peripherals_mouse
	 * Specifies all supported mouse buttons within Topaz.
	 */
	enum class MouseButton
	{
		Left,
		Right,
		Middle,
		Extra0,
		Extra1,
		Extra2,
		Extra3,
		Extra4,
		
		Count
	};

	struct MouseButtonInfo
	{
		const char* button_name;
		MouseButton button;

		bool operator==(const MouseButtonInfo& button) const {return this->button == button.button;}
	};

	namespace peripherals::mouse
	{
		/**
		 * @ingroup tz_core_peripherals_mouse
		 * Represents the null mouse button.
		 */
		constexpr MouseButtonInfo mouse_button_null = {.button_name = "Invalid", .button = MouseButton::Count};

		namespace detail
		{
			constexpr std::array<MouseButtonInfo, static_cast<int>(MouseButton::Count)> mouse_buttons
			{{
				{.button_name = "Left Mouse Button", .button = MouseButton::Left},
				{.button_name = "Right Mouse Button", .button = MouseButton::Right},
				{.button_name = "Middle Mouse Button", .button = MouseButton::Middle},
				{.button_name = "Extra Mouse Button 0", .button = MouseButton::Extra0},
				{.button_name = "Extra Mouse Button 1", .button = MouseButton::Extra1},
				{.button_name = "Extra Mouse Button 2", .button = MouseButton::Extra2},
				{.button_name = "Extra Mouse Button 3", .button = MouseButton::Extra3},
				{.button_name = "Extra Mouse Button 4", .button = MouseButton::Extra4},
			}};
		}

		/**
		 * @ingroup tz_core_peripherals_mouse
		 * Retrieve information about a specific mouse button. If the mouse button type is not supported or has an invalid value, the null mouse button is returned. See @ref mouse_button_null.
		 */
		constexpr MouseButtonInfo get_mouse_button(MouseButton button)
		{
			for(std::size_t i = 0; i < static_cast<int>(MouseButton::Count); i++)
			{
				if(detail::mouse_buttons[i].button == button)
				{
					return detail::mouse_buttons[i];
				}
			}
			return mouse_button_null;
		}
	}

	/**
	 * @ingroup tz_core_peripherals_mouse
	 * Stores all state for a mouse. Can be used to query which mouse buttons are currently pressed.
	 */
	class MouseButtonState
	{
	public:
		MouseButtonState() = default;
		/**
		 * Update mouse button state. This should be invoked each time the input backend receives a mouse press event.
		 * @param button Describes which mouse button sent an event.
		 * @param pressed Whether the button is pressed (true) or released (false).
		 */
		void update(MouseButtonInfo button, bool pressed);
		/**
		 * Query as to whether a mouse button is currently down. A mouse button is down if it has been pressed at some point in the past, but not yet released.
		 * @param button Describes which mouse button should be checked.
		 * @return True if mouse button is down, otherwise false.
		 */
		bool is_mouse_button_down(MouseButtonInfo button) const;
		/**
		 * Attempt to print entire mouse button state to a single line of stdout.
		 * 
		 * If `!TZ_DEBUG`, this does nothing at all.
		 */
		void debug_print_state() const;
	private:
		std::vector<MouseButtonInfo> pressed_buttons = {};
	};
}

#endif // TOPAZ_CORE_PERIPHERALS_MOUSE_HPP
