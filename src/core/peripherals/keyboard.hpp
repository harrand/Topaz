#ifndef TOPAZ_CORE_PERIPHERALS_KEYBOARD_HPP
#define TOPAZ_CORE_PERIPHERALS_KEYBOARD_HPP
#include <array>
#include <cctype>
#include <string>
#include <vector>

namespace tz
{
	/**
	 * @ingroup tz_core_peripherals
	 * @defgroup tz_core_peripherals_keyboard Keyboards
	 * Documentation for keyboard and key properties.
	 */
	
	/**
	 * @ingroup tz_core_peripherals_keyboard
	 * Specifies all supported key codes within Topaz.
	 */
	enum class KeyCode
	{
		Space, Backspace,
		Apostrophe,
		Comma, Period,
		Minus, Plus,
		ForwardSlash, BackSlash,
		Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
		Semicolon,
		Equals,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		LeftBracket, RightBracket,
		LeftParenthesis, RightParenthesis,
		LeftBrace, RightBrace,
		Grave,
		Escape,
		Enter,
		Tab,
		Insert,
		Delete,
		ArrowUp,
		ArrowLeft,
		ArrowDown,
		ArrowRight,
		PageUp,
		PageDown,
		Home,
		End,
		CapsLock, ScrollLock, NumLock,
		PrintScreen,
		Pause,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
		KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd, KeypadEquals,
		LeftControl, RightControl,
		LeftShift, RightShift,
		LeftAlt, AltGr,

		Count
	};

	/**
	 * @ingroup tz_core_peripherals_keyboard
	 * Describes a key on the keyboard.
	 */
	struct KeyInfo
	{
		/// Name of the key.
		const char* key_name;
		/// Unique key-code for the key.
		KeyCode code;
		/// Representation of the key, if it is typed. Many keys do not display a single character, for which this will simply be '\0'
		char representation = '\0';
	};

	namespace detail::peripherals::keyboard
	{
		/**
		 * @ingroup tz_core_peripherals_keyboard
		 * Represents the null key.
		 */
		constexpr KeyInfo key_null = {.key_name = "Invalid", .code = KeyCode::Count};

		constexpr std::array<KeyInfo, static_cast<int>(KeyCode::Count)> key_codes
		{{
			{.key_name = "Space", .code = KeyCode::Space, .representation = ' '},
			{.key_name = "Backspace", .code = KeyCode::Backspace},
			{.key_name = "Comma", .code = KeyCode::Comma, .representation = ','},
			{.key_name = "Period", .code = KeyCode::Period, .representation = '.'},
			{.key_name = "Minus", .code = KeyCode::Minus, .representation = '-'},
			{.key_name = "Plus", .code = KeyCode::Plus, .representation = '+'},
			{.key_name = "Forward Slash", .code = KeyCode::ForwardSlash, .representation = '/'},
			{.key_name = "Back Slash", .code = KeyCode::BackSlash, .representation = '\\'},
			{.key_name = "Zero", .code = KeyCode::Zero, .representation = '0'},
			{.key_name = "One", .code = KeyCode::One, .representation = '1'},
			{.key_name = "Two", .code = KeyCode::Two, .representation = '2'},
			{.key_name = "Three", .code = KeyCode::Three, .representation = '3'},
			{.key_name = "Four", .code = KeyCode::Four, .representation = '4'},
			{.key_name = "Five", .code = KeyCode::Five, .representation = '5'},
			{.key_name = "Six", .code = KeyCode::Six, .representation = '6'},
			{.key_name = "Seven", .code = KeyCode::Seven, .representation = '7'},
			{.key_name = "Eight", .code = KeyCode::Eight, .representation = '8'},
			{.key_name = "Nine", .code = KeyCode::Nine, .representation = '9'},
			{.key_name = "Semi-Colon", .code = KeyCode::Semicolon, .representation = ';'},
			{.key_name = "Equals", .code = KeyCode::Equals, .representation = '='},
			{.key_name = "A", .code = KeyCode::A, .representation = 'A'},
			{.key_name = "B", .code = KeyCode::B, .representation = 'B'},
			{.key_name = "C", .code = KeyCode::C, .representation = 'C'},
			{.key_name = "D", .code = KeyCode::D, .representation = 'D'},
			{.key_name = "E", .code = KeyCode::E, .representation = 'E'},
			{.key_name = "F", .code = KeyCode::F, .representation = 'F'},
			{.key_name = "G", .code = KeyCode::G, .representation = 'G'},
			{.key_name = "H", .code = KeyCode::H, .representation = 'H'},
			{.key_name = "I", .code = KeyCode::I, .representation = 'I'},
			{.key_name = "J", .code = KeyCode::J, .representation = 'J'},
			{.key_name = "K", .code = KeyCode::K, .representation = 'K'},
			{.key_name = "L", .code = KeyCode::L, .representation = 'L'},
			{.key_name = "M", .code = KeyCode::M, .representation = 'M'},
			{.key_name = "N", .code = KeyCode::N, .representation = 'N'},
			{.key_name = "O", .code = KeyCode::O, .representation = 'O'},
			{.key_name = "P", .code = KeyCode::P, .representation = 'P'},
			{.key_name = "Q", .code = KeyCode::Q, .representation = 'Q'},
			{.key_name = "R", .code = KeyCode::R, .representation = 'R'},
			{.key_name = "S", .code = KeyCode::S, .representation = 'S'},
			{.key_name = "T", .code = KeyCode::T, .representation = 'T'},
			{.key_name = "U", .code = KeyCode::U, .representation = 'U'},
			{.key_name = "V", .code = KeyCode::V, .representation = 'V'},
			{.key_name = "W", .code = KeyCode::W, .representation = 'W'},
			{.key_name = "X", .code = KeyCode::X, .representation = 'X'},
			{.key_name = "Y", .code = KeyCode::Y, .representation = 'Y'},
			{.key_name = "Z", .code = KeyCode::Z, .representation = 'Z'},
			{.key_name = "Left Bracket", .code = KeyCode::LeftBracket, .representation = '['},
			{.key_name = "Right Bracket", .code = KeyCode::RightBracket, .representation = ']'},
			{.key_name = "Left Parenthesis", .code = KeyCode::LeftParenthesis, .representation = '('},
			{.key_name = "Right Parenthesis", .code = KeyCode::RightParenthesis, .representation = ')'},
			{.key_name = "Left Brace", .code = KeyCode::LeftBrace, .representation = '{'},
			{.key_name = "Right Brace", .code = KeyCode::RightBrace, .representation = '}'},
			{.key_name = "Grave", .code = KeyCode::Grave, .representation = '`'},
			{.key_name = "Escape", .code = KeyCode::Escape},
			{.key_name = "Enter", .code = KeyCode::Enter, .representation = '\n'},
			{.key_name = "Tab", .code = KeyCode::Tab, .representation = '\t'},
			{.key_name = "Insert", .code = KeyCode::Insert},
			{.key_name = "Delete", .code = KeyCode::Delete},
			{.key_name = "Arrow Up", .code = KeyCode::ArrowUp},
			{.key_name = "Arrow Left", .code = KeyCode::ArrowLeft},
			{.key_name = "Arrow Down", .code = KeyCode::ArrowDown},
			{.key_name = "Arrow Right", .code = KeyCode::ArrowRight},
			{.key_name = "Page Up", .code = KeyCode::PageUp},
			{.key_name = "Page Down", .code = KeyCode::PageDown},
			{.key_name = "Home", .code = KeyCode::Home},
			{.key_name = "End", .code = KeyCode::End},
			{.key_name = "Caps Lock", .code = KeyCode::CapsLock},
			{.key_name = "Scroll Lock", .code = KeyCode::ScrollLock},
			{.key_name = "Num Lock", .code = KeyCode::NumLock},
			{.key_name = "Print Screen", .code = KeyCode::PrintScreen},
			{.key_name = "Pause", .code = KeyCode::Pause},
			{.key_name = "F1", .code = KeyCode::F1},
			{.key_name = "F2", .code = KeyCode::F2},
			{.key_name = "F3", .code = KeyCode::F3},
			{.key_name = "F4", .code = KeyCode::F4},
			{.key_name = "F5", .code = KeyCode::F5},
			{.key_name = "F6", .code = KeyCode::F6},
			{.key_name = "F7", .code = KeyCode::F7},
			{.key_name = "F8", .code = KeyCode::F8},
			{.key_name = "F9", .code = KeyCode::F9},
			{.key_name = "F10", .code = KeyCode::F10},
			{.key_name = "F11", .code = KeyCode::F11},
			{.key_name = "F12", .code = KeyCode::F12},
			{.key_name = "Keypad 0", .code = KeyCode::Keypad0},
			{.key_name = "Keypad 1", .code = KeyCode::Keypad1},
			{.key_name = "Keypad 2", .code = KeyCode::Keypad2},
			{.key_name = "Keypad 3", .code = KeyCode::Keypad3},
			{.key_name = "Keypad 4", .code = KeyCode::Keypad4},
			{.key_name = "Keypad 5", .code = KeyCode::Keypad5},
			{.key_name = "Keypad 6", .code = KeyCode::Keypad6},
			{.key_name = "Keypad 7", .code = KeyCode::Keypad7},
			{.key_name = "Keypad 8", .code = KeyCode::Keypad8},
			{.key_name = "Keypad 9", .code = KeyCode::Keypad9},
			{.key_name = "Keypad Decimal", .code = KeyCode::KeypadDecimal},
			{.key_name = "Keypad Divide", .code = KeyCode::KeypadDivide},
			{.key_name = "Keypad Multiply", .code = KeyCode::KeypadMultiply},
			{.key_name = "Keypad Subtract", .code = KeyCode::KeypadSubtract},
			{.key_name = "Keypad Add", .code = KeyCode::KeypadAdd},
			{.key_name = "Keypad Equals", .code = KeyCode::KeypadEquals},
			{.key_name = "Left Control", .code = KeyCode::LeftControl},
			{.key_name = "Right Control", .code = KeyCode::RightControl},
			{.key_name = "Left Shift", .code = KeyCode::LeftShift},
			{.key_name = "Right Shift", .code = KeyCode::RightShift},
			{.key_name = "Left Alt", .code = KeyCode::LeftAlt},
			{.key_name = "Alt Gr", .code = KeyCode::AltGr},
		}};

		/**
		 * @ingroup tz_core_peripherals_keyboard
		 * Retrieve information about the provided KeyCode. If the key-code is somehow not recognised, the null-key is returned. See @ref key_null.
		 * @return Key information.
		 */
		constexpr KeyInfo get_key(KeyCode code)
		{
			for(std::size_t i = 0; i < static_cast<int>(KeyCode::Count); i++)
			{
				if(key_codes[i].code == code)
				{
					return key_codes[i];
				}
			}
			return key_null;
		}

		constexpr char static_tolower_dirty(char c)
		{
			return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
		}

		/**
		 * @ingroup tz_core_peripherals_keyboard
		 * Attempt to retrieve the key information corresponding to a typed character. If the char does not correspond to a key, the null-key is instead returned. See @ref key_null.
		 *
		 * For example, `get_key('A').code == KeyCode::A`
		 */
		constexpr KeyInfo get_key(char representation)
		{
			representation = static_tolower_dirty(representation);
			for(std::size_t i = 0; i < static_cast<int>(KeyCode::Count); i++)
			{
				if(static_tolower_dirty(key_codes[i].representation) == representation)
				{
					return key_codes[i];
				}
			}
			return key_null;
		}
	}

	/**
	 * @ingroup tz_core_peripherals_keyboard
	 * Describes the nature of a keyboard key press.
	 */
	enum class KeyPressType
	{
		/// Key has just been pressed.
		Press,
		/// Key has just been released.
		Release,
		/// Key has been pressed for long enough to be considered 'repeating'.
		Repeat
	};

	/**
	 * @ingroup tz_core_peripherals_keyboard
	 * Describes information about a key press.
	 */
	struct KeyPressInfo
	{
		/// Information about which key was pressed.
		KeyInfo key;
		/// Nature of the key press.
		KeyPressType type;
	};

	/**
	 * @ingroup tz_core_peripherals_keyboard
	 * Stores all state for a keyboard. Can be used to query as to whether keys have been pressed or not.
	 */
	class KeyboardState
	{
	public:
		KeyboardState() = default;
		/**
		 * Update keyboard state. This should be invoked each time the input backend receives a key press event.
		 * @param info Describes the key press event.
		 */
		void update(KeyPressInfo info);
		/**
		 * Query as to whether a key is currently down. A key is down if it has been pressed at some point in the past, but not yet released.
		 * @param key Describes which key should be checked.
		 * @return True if key is down, otherwise false.
		 */
		bool is_key_down(KeyInfo key) const;
		/**
		 * Query as to whether a key has been pressed long enough for it to be considered 'repeating'. A key that is repeating is also considered to be down.
		 * @param key Describes which key should be checked.
		 * @return True if key is repeating, otherwise false.
		 */
		bool is_key_repeating(KeyInfo key) const;
		/**
		 * Attempt to print entire keyboard state to a single line of stdout.
		 * 
		 * If `!TZ_DEBUG`, this does nothing at all.
		 */
		void debug_print_state() const;
	private:
		std::vector<KeyPressInfo> pressed_keys = {};
	};
}

#endif // TOPAZ_CORE_PERIPHERALS_KEYBOARD_HPP
