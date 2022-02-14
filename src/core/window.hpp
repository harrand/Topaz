#ifndef TOPAZ_CORE_WINDOW_HPP
#define TOPAZ_CORE_WINDOW_HPP
#include "core/containers/basic_list.hpp"
#include "core/window_functionality.hpp"

namespace tz
{
	struct WindowHint
	{
		int hint;
		int value;
	};

	/**
	 * @ingroup tz_core_windowing
	 * Structure containing extra functionality for windows upon creation.
	 */
	struct WindowExtraFlags
	{
		/// If true, the window is resizeable, otherwise the window cannot be resized. Defaults to true.
		bool resizeable = true;
	};

	/**
	 * @ingroup tz_core_windowing
	 * Specifies creation flags for a window.
	 */
	struct WindowInitArgs
	{
		/// Width of the window, in pixels. Defaults to 800.
		int width = 800;
		/// Height of the window, in pixels. Defaults to 600.
		int height = 600;
		/// Title of the window. Defaults to "Untitled".
		const char* title = "Untitled";
		/// Extra functionality for the window. If you don't provide this, the defaults are used.
		WindowExtraFlags flags = {};
	};

	using WindowHintList = tz::BasicList<WindowHint>;

	/**
	 * @ingroup tz_core_windowing
	 * Represents an OS Window.
	 */
	class Window : public WindowFunctionality
	{
	public:
		/**
		 * Create a new Window.
		 */
		Window(WindowInitArgs args = {}, WindowHintList hints = {});
		Window(const Window& copy) = delete;
		Window(Window&& move);
		~Window();
		Window& operator=(const Window& rhs) = delete;
		Window& operator=(Window&& rhs);

		/**
		 * Retrieve the null window. Operations on the null window are invalid, but it is a useful alias if the application is headless.
		 */
		static Window null();
		/**
		 * Query as to whether the window is the null window. See @ref Window::null().
		 */
		bool is_null() const;

		static void window_resize_callback(GLFWwindow* window, int width, int height);
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	private:
		void handle_key_event(int key, int scancode, int action, int mods);
		
		Window(std::nullptr_t);
	};
}

#endif // TOPAZ_CORE_WINDOW_HPP
