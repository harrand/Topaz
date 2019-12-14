//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_GLFW_CONTEXT_HPP
#define TOPAZ_GLFW_CONTEXT_HPP
#include "core/tz_glfw/glfw_window.hpp"
#include <optional>

namespace tz::ext::glfw
{
	/**
	 * Wrapper for interacting with GLFW.
	 * GLFW is a multi-platform library for OpenGL development.
	 * Topaz uses this for input listening and windowing.
	 * It is impossible to invoke any OpenGL functions without first loading them via this context.
	 * GLFW contexts always come with their own GLFW window at present. This is intended to be relaxed in the future.
	 */
	class GLFWContext
	{
	public:
		/**
		 * Construct an empty, unused GLFWContext
		 */
		GLFWContext() noexcept;
		/**
		 * Initialise the context.
		 * This will assert if you invoke initialise more than once without invoking term() between.
		 * Note: It is fine to init(), term() and invoke init() again and continue rendering.
		 */
		void init();
		/**
		 * Terminate the context.
		 * This will assert if you terminate more than once without invoking init() between.
		 * This will assert if you terminate without invoking init() prior.
		 * Note: It is fine to init(), term() and invoke init() again and continue rendering.
		 */
		void term();
		/**
		 * Get a pointer to the window implementation if we were assigned one.
		 * This will only happen if tz::core has constructed this context.
		 * @return - Pointer to the window implementation if there is one, otherwise nullptr
		 */
		tz::ext::glfw::GLFWWindowImpl* get_window();
		/**
		 * Get a pointer to the window implementation if we were assigned one.
		 * This will only happen if tz::core has constructed this context.
		 * @return - Pointer to the window implementation if there is one, otherwise nullptr
		 */
		const tz::ext::glfw::GLFWWindowImpl* get_window() const;
		/**
		 * Query as to whether this context has an accompanying window implementation.
		 * This will be true if tz::core has constructed this context.
		 * If you constructed this context yourself, this is likely to be false.
		 * @return - True if there is a window implementation, otherwise false
		 */
		bool has_window() const;
		/**
		 * Query as to whether this GLFW context is currently the active context.
		 * Only one GLFW context (with an accompanying window) can be active at a time.
		 * @return - True if this context is the active context, otherwise false
		 */
		bool is_active_context() const;
		
		/**
		 * TODO: Document
		 * @param rhs
		 * @return
		 */
		bool operator==(const GLFWContext& rhs) const;
	
		// Is it a good idea to have tight coupling due to this?
		friend void give_window(GLFWContext&, GLFWWindowImpl&&);
		friend GLFWContext& make_secondary_context(WindowCreationArgs);
		friend class tz::core::GLFWWindow;
	private:
		explicit GLFWContext(bool pre_initialised) noexcept;
		/**
		 * Take ownership of an existing window implementation.
		 * @param window - Window implementation to foster
		 */
		void set_window(tz::ext::glfw::GLFWWindowImpl&& window);

		/// Have we been initialised?
		bool initialised;
		/// What args were we given for our window? (e.g title, width, height)
		tz::ext::glfw::WindowCreationArgs args;
		/// Stores the window implementation. There may not be one.
		std::optional<tz::ext::glfw::GLFWWindowImpl> window;
	};

	/**
	 * Initialize a GLFW context and create a window.
	 * This must be done aswell as loading OpenGL before rendering can be done.
	 * The window will be available via tz::ext::glfw::get().get_window().
	 * @param args - Arguments with which to create the window
	 */
	void initialise(WindowCreationArgs args);
	/**
	 * Destroy the active window and terminate the underlying context.
	 */
	void terminate();
	/**
	 * Retrieve the current context.
	 * Note: This is not necessarily the same as the active context.
	 * @return - Reference to the active GLFW context
	 */
	GLFWContext& get();
	/**
	 * TODO: Document
	 * @return
	 */
	GLFWContext& make_secondary_context(WindowCreationArgs args);
	/**
	 * TODO: Document
	 * @param ctx
	 */
	void destroy_secondary_context(GLFWContext& ctx);
}

#endif //TOPAZ_GLFW_CONTEXT_HPP
