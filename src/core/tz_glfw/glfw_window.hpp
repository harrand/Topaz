//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_GLFW_WINDOW_HPP
#define TOPAZ_GLFW_WINDOW_HPP
#include <map>
// Forward declares
struct GLFWmonitor;
struct GLFWwindow;
namespace tz::core
{
    class GLFWWindow;
}
namespace tz::input
{
	struct KeyPressEvent;
	struct CharPressEvent;
}


namespace tz::ext::glfw
{
	/**
	 * Brief information about what a window might look like.
	 */
    struct WindowCreationArgs
    {
        WindowCreationArgs();
        WindowCreationArgs(const char* title, int width, int height);

        const char* title;
        int width;
        int height;
    };
	
	/**
	 * GLFW low-level window implementation.
	 * Here be programming dragons.
	 */
    class GLFWWindowImpl
    {
    public:
        // No copying, only moving allowed.
        GLFWWindowImpl(const GLFWWindowImpl& copy) = delete;
        GLFWWindowImpl(GLFWWindowImpl&& move) noexcept;
        ~GLFWWindowImpl();
        GLFWWindowImpl& operator=(const GLFWWindowImpl& copy) = delete;
        GLFWWindowImpl& operator=(GLFWWindowImpl&& move) noexcept;
		/**
		 * Allow tz_glfw to track this window implementation.
		 * Tracking is required to be able to send input data to listeners properly.
		 * @param window
		 */
		void register_this(tz::core::GLFWWindow* window);
		/**
		 * GLFW contexts are tightly-coupled with windows.
		 * Query as to whether this window (and its context) are currently the active context.
		 * @return - True if the context is active, otherwise false
		 */
		bool has_active_context() const;
	
		// Is it a good idea to have tight coupling due to this?
        friend void initialise(WindowCreationArgs);
        friend class tz::core::GLFWWindow;
    private:
		/**
		 * Construct the window implementation given some brief information.
		 * @param width - Width of the window, in pixels
		 * @param height - Height of the window, in pixels
		 * @param title - Title of the window
		 * @param monitor - Monitor implementation (nullable)
		 * @param share - Unknown (nullable)
		 */
        GLFWWindowImpl(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
		/**
		 * Construct the window implementation given some brief information.
		 * @param args - Structure containing window information, such as width, height and title
		 */
        explicit GLFWWindowImpl(WindowCreationArgs args);

		/// Underlying glfw window handle
        GLFWwindow* window_handle;
		/// What's the title of the window?
        const char* title;
    };

	namespace detail
	{
		/// Implementation detail. Stores all the tracking data for each active window. Module-wide.
		static std::map<GLFWwindow *, tz::core::GLFWWindow *> window_userdata;
	}
	
	/**
	 * Instruct tz_glfw to track this window's underlying implementation.
	 * This is required to use listeners.
	 * @param window - Window to track polling data for
	 */
	void register_window(tz::core::GLFWWindow* window);
	/**
	 * Invoked by anyone who wishes to send off an input command.
	 * You can invoke this directly if you wish to inject listener inputs artificially!
	 * Note: It's easier to use the simulate_key_press function below to inject listener input data.
	 * @param window - Window who is responsible for handling this input
	 * @param key - What key was mentioned? e.g GLFW_KEY_A
	 * @param scancode - Opaque handle for a given key; platform-specific
	 * @param action - In what way was the key operated? e.g GLFW_PRESS
	 * @param mods - Which modifier flags were used? e.g GLFW_MOD_SHIFT
	 */
	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	/**
	 * Invoked by anyone who wishes to send off an input command.
	 * You can invoke this directly if you wish to inject listener inputs artificially!
	 * Note: It's easier to use the simulate_key_type function below to inject data.
	 * Note: To inject strings, consider using simulate_typing.
	 * @param window - Window who is responsible for handling this input
	 * @param codepoint - Corresponds to a unicode code-point. Consider just static_cast'ing this to a char if you're using ASCII
	 */
	void glfw_char_callback(GLFWwindow* window, unsigned int codepoint);
	/**
	 * Pretend that a key-press occurred!
	 * This will be handled by the window whose context is currently active.
	 * To force the event to be handled by a specific inactive window, consider invoking glfw_key_callback above.
	 * @param kpe - Information about the key press
	 */
	void simulate_key_press(const tz::input::KeyPressEvent& kpe);
	/**
	 * Pretend that a character was typed!
	 * This will be handled by the window whose context is currently active.
	 * To force the event to be handled by a specific inactive window, consider invoking glfw_char_callback above.
	 * @param cpe - Information about the character typed
	 */
	void simulate_key_type(const tz::input::CharPressEvent& cpe);
	/**
	 * Pretend that multiple characters were typed!
	 * This will be handled by the window whose context is currently active.
	 * To force the events to be handled by a specific inactive window, consider invoking glfw_char_callback multiple times above.
	 * Note: This may be useful for automatically inputting text into various fields.
	 * @param letters - C-String corresponding to the characters typed in-order
	 */
	void simulate_typing(const char* letters);
}


#endif //TOPAZ_GLFW_WINDOW_HPP
