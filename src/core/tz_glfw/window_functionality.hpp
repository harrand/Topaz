#ifndef TOPAZ_CORE_TZ_GLFW_WINDOW_FUNCTIONALITY_HPP
#define TOPAZ_CORE_TZ_GLFW_WINDOW_FUNCTIONALITY_HPP
#include "core/window.hpp"
#include "gl/frame.hpp"
#include "GLFW/glfw3.h"
#include <utility>

// Forward declare
namespace tz::gl
{
    class IFrame;
}

namespace tz::ext::glfw
{
    class GLFWWindowFunctionality : public tz::IWindow
    {
    public:
        GLFWWindowFunctionality(GLFWwindow* window = nullptr);
        void set_handle(GLFWwindow* window);

        virtual const char* get_title() const override;
		virtual void set_title(const char* title) override;
		
		virtual int get_width() const override;
		virtual int get_height() const override;
		virtual void set_width(int width)override;
		virtual void set_height(int height) override;
		virtual void set_size(int width, int height) override;

		virtual tz::gl::IFrame* get_frame() const override;
		
		virtual bool is_visible() const override;
		virtual void set_visible(bool visible) override;
	
		virtual bool is_resizeable() const override;
		virtual void set_resizeable(bool resizeable) override;
	
		virtual bool is_focused() const override;
		virtual void set_focused(bool focused) override;
		
		virtual bool is_close_requested() const override;
		virtual void request_close() override;
		
		virtual void set_active_context() override;
		virtual bool is_active_context() const override;

		virtual bool is_cursor_showing() const override;
		virtual void set_cursor_showing(bool show_cursor) override;
		
		virtual void update() const override;
		virtual void handle_key_event(const tz::input::KeyPressEvent& kpe) override;
		virtual void handle_type_event(const tz::input::CharPressEvent& cpe) override;
		virtual void handle_mouse_event(const tz::input::MouseUpdateEvent& pos) override;
		virtual void handle_click_event(const tz::input::MouseClickEvent& click) override;
		
		virtual void register_key_listener(std::unique_ptr<tz::input::KeyListener> listener) override;
		virtual void unregister_key_listener(std::unique_ptr<tz::input::KeyListener> listener) override;
	
		virtual void register_type_listener(std::unique_ptr<tz::input::TypeListener> listener) override;
		virtual void unregister_type_listener(std::unique_ptr<tz::input::TypeListener> listener) override;

		virtual void register_mouse_listener(std::unique_ptr<tz::input::MouseListener> listener) override;
		virtual void unregister_mouse_listener(std::unique_ptr<tz::input::MouseListener> listener) override;
    protected:
        GLFWwindow* window_handle;
    private:
        std::pair<int, int> get_size() const;
        int get_attribute(int attrib_type) const;
        void set_attribute(int attrib_type, int value);
        std::string title;
        std::unique_ptr<tz::gl::IFrame> frame;
        std::vector<std::unique_ptr<tz::input::KeyListener>> key_listeners;
        std::vector<std::unique_ptr<tz::input::TypeListener>> type_listeners;
        std::vector<std::unique_ptr<tz::input::MouseListener>> mouse_listeners;
    };
}

#endif // TOPAZ_CORE_TZ_GLFW_WINDOW_FUNCTIONALITY_HPP