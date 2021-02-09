//
// Created by Harrand on 13/12/2019.
//

#include "window.hpp"
#include "core/debug/assert.hpp"
#include "gl/frame.hpp"
#include "GLFW/glfw3.h"
#include <algorithm>

namespace tz
{
	GLFWWindow::GLFWWindow(tz::ext::glfw::GLFWContext& ctx): impl(ctx.get_window()), key_listeners(), type_listeners(), mouse_listeners()
	{
		this->set_active_context();
	}

	const char* GLFWWindow::get_title() const
	{
		this->verify();
		return this->impl->get_title().c_str();
	}
	
	void GLFWWindow::set_title(const char* title)
	{
		this->verify();
		this->impl->set_title(title);
	}
	
	int GLFWWindow::get_width() const
	{
		return this->get_size().first;
	}
	
	int GLFWWindow::get_height() const
	{
		return this->get_size().second;
	}
	
	void GLFWWindow::set_width(int width) const
	{
		this->verify();
		this->impl->set_width(width);
	
	}
	void GLFWWindow::set_height(int height) const
	{
		this->verify();
		this->impl->set_height(height);
	}
	
	void GLFWWindow::set_size(int width, int height) const
	{
		this->verify();
		this->impl->set_size(width, height);
	}

	tz::gl::IFrame* GLFWWindow::get_frame() const
	{
		return this->impl->get_frame();
	}
	
	bool GLFWWindow::is_visible() const
	{
		this->verify();
		return static_cast<bool>(this->impl->get_attribute(GLFW_VISIBLE));
	}
	
	void GLFWWindow::set_visible(bool visible) const
	{
		this->verify();
		this->impl->set_attribute(GLFW_VISIBLE, visible);
	}
	
	bool GLFWWindow::is_resizeable() const
	{
		this->verify();
		return static_cast<bool>(this->impl->get_attribute(GLFW_RESIZABLE));
	}
	
	void GLFWWindow::set_resizeable(bool resizeable) const
	{
		this->verify();
		this->impl->set_attribute(GLFW_RESIZABLE, resizeable);
	}
	
	bool GLFWWindow::is_focused() const
	{
		this->verify();
		return static_cast<bool>(this->impl->get_attribute(GLFW_FOCUSED));
	}
	
	void GLFWWindow::set_focused(bool focused) const
	{
		this->verify();
		this->impl->set_attribute(GLFW_FOCUSED, focused);
	}
	
	bool GLFWWindow::is_close_requested() const
	{
		this->verify();
		return this->impl->close_requested();
	}
	
	void GLFWWindow::request_close() const
	{
		this->verify();
		this->impl->set_close_requested(true);
	
	}
	
	std::pair<int, int> GLFWWindow::get_size() const
	{
		this->verify();
		return this->impl->get_size();
	}
	
	void GLFWWindow::verify() const
	{
		#if TOPAZ_DEBUG
			topaz_assert(this->impl != nullptr, "GLFWWindow::verify(): Verification failed. Did you tz::initialise()?");
		#endif
	}

	void GLFWWindow::ensure_registered()
	{
		if(!this->impl->is_registered())
			this->register_this();
	}
	
	void GLFWWindow::set_active_context() const
	{
		this->verify();
		this->impl->set_active_context();
	}
	
	bool GLFWWindow::is_active_context() const
	{
		this->verify();
		return this->impl->has_active_context();
	}
	
	void GLFWWindow::update() const
	{
		this->verify();
		this->impl->swap_buffers();
	}
	
	void GLFWWindow::handle_key_event(const tz::input::KeyPressEvent& kpe)
	{
		for(const auto& listener_ptr : key_listeners)
		{
			listener_ptr->on_key_press(kpe);
		}
	}
	
	void GLFWWindow::handle_type_event(const tz::input::CharPressEvent& cpe)
	{
		for(const auto& listener_ptr : type_listeners)
		{
			listener_ptr->on_key_type(cpe);
		}
	}

	void GLFWWindow::handle_mouse_event(const tz::input::MouseUpdateEvent& pos)
	{
		for(const auto& listener_ptr : mouse_listeners)
		{
			listener_ptr->on_mouse_update(pos);
		}
	}

	void GLFWWindow::handle_click_event(const tz::input::MouseClickEvent& click)
	{
		for(const auto& listener_ptr : mouse_listeners)
		{
			listener_ptr->on_mouse_click(click);
		}
	}
	
	void GLFWWindow::register_key_listener(std::shared_ptr<tz::input::KeyListener> listener)
	{
		this->ensure_registered();
		// Don't add duplicates -- That would be very annoying.
		if(std::find(this->key_listeners.begin(), this->key_listeners.end(), listener) != this->key_listeners.end())
		{
			return;
		}
		this->key_listeners.push_back(listener);
	}
	
	void GLFWWindow::unregister_key_listener(std::shared_ptr<tz::input::KeyListener> listener)
	{
		this->key_listeners.erase(std::remove(this->key_listeners.begin(), this->key_listeners.end(), listener), this->key_listeners.end());
	}
	
	void GLFWWindow::register_type_listener(std::shared_ptr<tz::input::TypeListener> listener)
	{
		this->ensure_registered();
		this->type_listeners.push_back(listener);
	}
	
	void GLFWWindow::unregister_type_listener(std::shared_ptr<tz::input::TypeListener> listener)
	{
		this->type_listeners.erase(std::remove(this->type_listeners.begin(), this->type_listeners.end(), listener), this->type_listeners.end());
	}

	void GLFWWindow::register_mouse_listener(std::shared_ptr<tz::input::MouseListener> listener)
	{
		this->ensure_registered();
		this->mouse_listeners.push_back(listener);
	}

	void GLFWWindow::unregister_mouse_listener(std::shared_ptr<tz::input::MouseListener> listener)
	{
		this->mouse_listeners.erase(std::remove(this->mouse_listeners.begin(), this->mouse_listeners.end(), listener), this->mouse_listeners.end());
	}
	
	void GLFWWindow::register_this()
	{
		this->impl->register_this(this);
	}
}