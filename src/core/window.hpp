//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_WINDOW_HPP
#define TOPAZ_WINDOW_HPP
#include "core/tz_glfw/glfw_context.hpp"
#include <memory>
#include <vector>

// Forward declares
namespace tz
{
	namespace input
	{
		struct KeyListener;
		struct TypeListener;
		struct MouseListener;

		struct KeyPressEvent;
		struct CharPressEvent;
		struct MouseUpdateEvent;
		struct MouseClickEvent;
	}
}

namespace tz::core
{
	/**
	 * Interface for a high-level implementation of a cross-platform window.
	 */
    class IWindow
    {
    public:
		/**
		 * Retrieve the title of the window.
		 * @return - Title of the window
		 */
        virtual const char* get_title() const = 0;
		/**
		 * Give the window a new title.
		 * @param title - Desired new title of the window
		 */
        virtual void set_title(const char* title) = 0;
		
		/**
		 * Get the width of the window, in pixels.
		 * @return - Width in pixels
		 */
        virtual int get_width() const = 0;
		/**
		 * Get the height of the window, in pixels.
		 * @return - Height in pixels
		 */
        virtual int get_height() const = 0;
		/**
		 * Set the width of the window to a new value, in pixels.
		 * @param width - Desired new width in pixels
		 */
        virtual void set_width(int width) const = 0;
		/**
		 * Set the height of the window to a new value, in pixels.
		 * @param height - Desired new height in pixels
		 */
        virtual void set_height(int height) const = 0;
		/**
		 * Set both the width and height of the window at once. Both values are in pixels.
		 * @param width - Width in pixels
		 * @param height - Height in pixels
		 */
		virtual void set_size(int width, int height) const = 0;
		
		/**
		 * Get the underlying tz::gl::Frame corresponding to this window.
		 * 
		 * Note: Use this to set the window as a render-target.
		 * @return Frame corresponding to this window.
		 */
		virtual tz::gl::IFrame* get_frame() const = 0;

		/**
		 * Query as to whether the window is currently visible to the user.
		 * Note: This will return true if the window is technically visible but occluded.
		 * @return - True if the user should be able to see the window, otherwise false
		 */
        virtual bool is_visible() const = 0;
		/**
		 * Set whether the window should be visible to the user or not.
		 * @param visible - Whether the window should be visible
		 */
        virtual void set_visible(bool visible) const = 0;
		
		/**
		 * Query as to whether the window is currently able to be resized by the user.
		 * @return - True if the user can resize the window, otherwise false
		 */
		virtual bool is_resizeable() const = 0;
		/**
		 * Set whether the user should be able to resize the window.
		 * @param resizeable - Whether the window should be resizeable by the user
		 */
		virtual void set_resizeable(bool resizeable) const = 0;
		
		/**
		 * Query as to whether the window is currently in-focus.
		 * @return - True if the window is focused, otherwise false
		 */
		virtual bool is_focused() const = 0;
		/**
		 * Set whether the window should be focused or not.
		 * @param focused - Whether the window should be focused
		 */
		virtual void set_focused(bool focused) const = 0;
		
		/**
		 * Query as to whether the user has requested the window to close.
		 * It is expected that the window be destroyed the frame after it was requested to close.
		 * @return - True if the window is about to close, otherwise false
		 */
        virtual bool is_close_requested() const = 0;
		/**
		 * Request that the window close as soon as possible.
		 * This is likely to be at the end of the next frame.
		 */
        virtual void request_close() const = 0;
	
		/**
		 * Set the underlying GL context to be the active context.
		 * This means that subsequent OpenGL operations will apply to this window.
		 */
		virtual void set_active_context() const = 0;
		/**
		 * Query as to whether this window's underlying GL context is currently the active context.
		 * @return - True if this window has the active context, otherwise false
		 */
		virtual bool is_active_context() const = 0;
		
		/**
		 * Request that the window swap buffers.
		 */
		virtual void update() const = 0;
		/**
		 * Send a key-press event to the window.
		 * @param kpe - Information about the key-press
		 */
		virtual void handle_key_event(const tz::input::KeyPressEvent& kpe) = 0;
		/**
		 * Send a character-typed event to the window.
		 * @param cpe - Information about the character typed
		 */
		virtual void handle_type_event(const tz::input::CharPressEvent& cpe) = 0;
		/**
		 * TODO: Document
		 * @param pos
		 */
		virtual void handle_mouse_event(const tz::input::MouseUpdateEvent& pos) = 0;
		/**
		 * TODO: Document
		 * @param click
		 */
		virtual void handle_click_event(const tz::input::MouseClickEvent& click) = 0;
	
		/**
		 * Register a new key-press listener. The new listener will be notified of any events received by the window.
		 * Note: If the listener was already registered by this window, this method has no effect.
		 * @param listener - Allocated listener that this window should share ownership of and inform of events
		 */
		virtual void register_key_listener(std::shared_ptr<tz::input::KeyListener> listener) = 0;
		/**
		 * Unregister an existing key-press listener. The listener will no longer be notified of any events received by the window.
		 * Note: If the listener wasn't previously registered by this window, this method has no effect.
		 * @param listener - Allocated listener that the window will no longer share ownership of
		 */
		virtual void unregister_key_listener(std::shared_ptr<tz::input::KeyListener> listener) = 0;
	
		/**
		 * Register a new character-typed listener. The new listener will be notified of any events received by the window.
		 * Note: If the listener was already registered by this window, this method has no effect.
		 * @param listener - Allocated listener that this window should share ownership of and inform of events
		 */
		virtual void register_type_listener(std::shared_ptr<tz::input::TypeListener> listener) = 0;
		/**
		 * Unregister an existing character-typed listener. The listener will no longer be notified of any events received by the window.
		 * Note: If the listener wasn't previously registered by this window, this method has no effect.
		 * @param listener - Allocated listener that the window will no longer share ownership of
		 */
		virtual void unregister_type_listener(std::shared_ptr<tz::input::TypeListener> listener) = 0;
		/**
		 * TODO: Document
		 * @param listener - Allocated listener that this window should share ownership of and inform of events
		 */
		virtual void register_mouse_listener(std::shared_ptr<tz::input::MouseListener> listener) = 0;
		/**
		 * TODO: Document
		 * @param listener - Allocated listener that this window should share ownership of and inform of events
		 */
		virtual void unregister_mouse_listener(std::shared_ptr<tz::input::MouseListener> listener) = 0;
		
		/**
		 * Construct a new key-pressed listener in-place and register it for this window.
		 * Note: This is expected to be a CustomKeyListener<T>.
		 * @tparam T - Listener callback type. This is likely to be a lambda type
		 * @tparam Args - Types of additional arguments used to construct the listener
		 * @param callback - Callback value to be invoked when an event is received
		 * @param args - Additional argument values required to construct the listener
		 * @return - Reference to the constructed listener
		 */
		template<typename T, typename... Args>
		tz::input::KeyListener& emplace_custom_key_listener(T callback, Args&&... args);
		/**
		 * Construct a new character-typed listener in-place and register it for this window.
		 * Note: This is expected to be a CustomTypeListener<T>.
		 * @tparam T - Listener callback type. This is likely to be a lambda type
		 * @tparam Args - Types of additional arguments used to construct the listener
		 * @param callback - Callback value to be invoked when an event is received
		 * @param args - Additional argument values required to construct the listener
		 * @return - Reference to the constructed listener
		 */
		template<typename T, typename... Args>
		tz::input::TypeListener& emplace_custom_type_listener(T callback, Args&&... args);
		/**
		 * TODO: Document
		 */
		template<typename TUpdate, typename TClick, typename... Args>
		tz::input::MouseListener& emplace_custom_mouse_listener(TUpdate update, TClick click, Args&&... args);
		/**
		 * Register the underlying window implementation to be tracked by topaz.
		 * This is required in order to receive input events.
		 */
		virtual void register_this(){}
	};


    class GLFWWindow : public IWindow
    {
    public:
        explicit GLFWWindow(tz::ext::glfw::GLFWContext& context);
        virtual const char* get_title() const override;
        virtual void set_title(const char* title) override;
		
        virtual int get_width() const override;
        virtual int get_height() const override;
        virtual void set_width(int width) const override;
        virtual void set_height(int height) const override;
		virtual void set_size(int width, int height) const override;

		virtual tz::gl::IFrame* get_frame() const override;
		
        virtual bool is_visible() const override;
        virtual void set_visible(bool visible) const override;
	
		virtual bool is_resizeable() const override;
		virtual void set_resizeable(bool resizeable) const override;
	
		virtual bool is_focused() const override;
		virtual void set_focused(bool focused) const override;
		
        virtual bool is_close_requested() const override;
        virtual void request_close() const override;
		
		virtual void set_active_context() const override;
		virtual bool is_active_context() const override;
		
		virtual void update() const override;
		virtual void handle_key_event(const tz::input::KeyPressEvent& kpe) override;
		virtual void handle_type_event(const tz::input::CharPressEvent& cpe) override;
		virtual void handle_mouse_event(const tz::input::MouseUpdateEvent& pos) override;
		virtual void handle_click_event(const tz::input::MouseClickEvent& click) override;
		
		virtual void register_key_listener(std::shared_ptr<tz::input::KeyListener> listener) override;
		virtual void unregister_key_listener(std::shared_ptr<tz::input::KeyListener> listener) override;
	
		virtual void register_type_listener(std::shared_ptr<tz::input::TypeListener> listener) override;
		virtual void unregister_type_listener(std::shared_ptr<tz::input::TypeListener> listener) override;

		virtual void register_mouse_listener(std::shared_ptr<tz::input::MouseListener> listener) override;
		virtual void unregister_mouse_listener(std::shared_ptr<tz::input::MouseListener> listener) override;
		
		virtual void register_this() override;
		
		friend void tz::ext::glfw::register_window(tz::core::GLFWWindow*);
    private:
		/**
		 * Retrieve the width and height of the window, in pixels.
		 * @return - {width, height} in pixels
		 */
		std::pair<int, int> get_size() const;
		/**
		 * Debug-only helper method to perform sanity-checking.
		 * Asserts if the window implementation is null.
		 */
		void verify() const;
		void ensure_registered();
		
		/// Underlying window implementation. This should not be null.
        tz::ext::glfw::GLFWWindowImpl* impl;
		/// Stores all registered key-listeners.
		std::vector<std::shared_ptr<tz::input::KeyListener>> key_listeners;
		/// Stores all registered type-listeners.
		std::vector<std::shared_ptr<tz::input::TypeListener>> type_listeners;
		/// Stores all registered mouse-listeners.
		std::vector<std::shared_ptr<tz::input::MouseListener>> mouse_listeners;
    };
    
    using Window = GLFWWindow;
}

#include "core/window.inl"
#endif //TOPAZ_WINDOW_HPP
