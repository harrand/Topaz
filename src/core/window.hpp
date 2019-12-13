//
// Created by Harrand on 13/12/2019.
//

#ifndef TOPAZ_WINDOW_HPP
#define TOPAZ_WINDOW_HPP
#include "core/tz_glfw/glfw_context.hpp"
#include <memory>
#include <vector>

// Forward declare
namespace tz::input
{
	struct KeyListener;
	struct TypeListener;
	struct KeyPressEvent;
	struct CharPressEvent;
}

namespace tz::core
{
    class IWindow
    {
    public:
        virtual const char* get_title() const = 0;
        virtual void set_title(const char* title) = 0;
		
        virtual int get_width() const = 0;
        virtual int get_height() const = 0;
        virtual void set_width(int width) const = 0;
        virtual void set_height(int height) const = 0;
		virtual void set_size(int width, int height) const = 0;
		
        virtual bool is_visible() const = 0;
        virtual void set_visible(bool visible) const = 0;
		
		virtual bool is_resizeable() const = 0;
		virtual void set_resizeable(bool resizeable) const = 0;
		
		virtual bool is_focused() const = 0;
		virtual void set_focused(bool focused) const = 0;
		
        virtual bool is_close_requested() const = 0;
        virtual void request_close() const = 0;
	
		virtual void set_active_context() const = 0;
		virtual bool is_active_context() const = 0;
		
		virtual void update() const = 0;
		virtual void handle_key_event(const tz::input::KeyPressEvent& kpe) = 0;
		virtual void handle_type_event(const tz::input::CharPressEvent& cpe) = 0;
	
		virtual void register_key_listener(std::shared_ptr<tz::input::KeyListener> listener) = 0;
		virtual void unregister_key_listener(std::shared_ptr<tz::input::KeyListener> listener) = 0;
	
		virtual void register_type_listener(std::shared_ptr<tz::input::TypeListener> listener) = 0;
		virtual void unregister_type_listener(std::shared_ptr<tz::input::TypeListener> listener) = 0;
		
		template<typename T, typename... Args>
		tz::input::KeyListener& emplace_custom_key_listener(T callback, Args&&... args);
		template<typename T, typename... Args>
		tz::input::TypeListener& emplace_custom_type_listener(T callback, Args&&... args);
		
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
		
		virtual void register_key_listener(std::shared_ptr<tz::input::KeyListener> listener) override;
		virtual void unregister_key_listener(std::shared_ptr<tz::input::KeyListener> listener) override;
	
		virtual void register_type_listener(std::shared_ptr<tz::input::TypeListener> listener) override;
		virtual void unregister_type_listener(std::shared_ptr<tz::input::TypeListener> listener) override;
		
		virtual void register_this() override;
		
		friend void tz::ext::glfw::register_window(tz::core::GLFWWindow*);
    private:
		std::pair<int, int> get_size() const;
		void verify() const;
		
        tz::ext::glfw::GLFWWindowImpl* impl;
		std::vector<std::shared_ptr<tz::input::KeyListener>> key_listeners;
		std::vector<std::shared_ptr<tz::input::TypeListener>> type_listeners;
    };
    
    using Window = GLFWWindow;
}

#include "core/window.inl"
#endif //TOPAZ_WINDOW_HPP
