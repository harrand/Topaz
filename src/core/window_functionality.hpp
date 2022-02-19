#ifndef TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
#define TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
#include "core/types.hpp"
#include "core/callback.hpp"
#include "core/peripherals/keyboard.hpp"
#include "core/peripherals/mouse.hpp"
#include <vector>
#include <functional>

#if TZ_VULKAN
	#define GLFW_INCLUDE_VULKAN
#endif // TZ_VULKAN
#include "GLFW/glfw3.h"

namespace tz
{
	using WindowResizeSignature = void(int width, int height);
	template<typename F>
	concept WindowResizeConcept = tz::Action<F, int, int>;

	class WindowFunctionality
	{
	public:
		using ResizeCallbackType = Callback<int, int>;
		WindowFunctionality(GLFWwindow* wnd);
		GLFWwindow* get_middleware_handle() const;
		/**
		 * Query as to whether the user has requested that the window should close.
		 */
		bool is_close_requested() const;
		/**
		 * Retrieve the width of the window, in pixels.
		 */
		float get_width() const;
		/**
		 * Retrieve the height of the window, in pixels.
		 */
		float get_height() const;
		/**
		 * Retrieve the callback object for when the window is resized.
		 */
		ResizeCallbackType& on_resize();

		/**
		 * Retrieve state of keyboard presses.
		 */
		const KeyboardState& get_keyboard_state() const;
		/**
		 * Retrieve state of mouse button presses.
		 */
		const MouseButtonState& get_mouse_button_state() const;

		/**
		 * Advance an update iteration for the window backend, polling for events etc...
		 *
		 * You should invoke this once for every frame.
		 */
		void update();

		/**
		 * Block the current thread until some sort of window backend event takes place.
		 *
		 * An example use-case for this is holding off on rendering after the window has been minimised (and not doing any until the window is maximised again or otherwise awoken).
		 */
		static void block_until_event_happens();
	protected:
		void handle_key_event(int key, int scancode, int action, int mods);
		void handle_mouse_event(int button, int action, int mods);

		GLFWwindow* wnd;
		ResizeCallbackType window_resize_callbacks;
	private:
		std::pair<int, int> get_size() const;
		void ensure() const;

		KeyboardState kb_state;
		MouseButtonState mb_state;
	};
}

#include "core/window_functionality.inl"
#endif // TOPAZ_CORE_WINDOW_FUNCTIONALITY_HPP
