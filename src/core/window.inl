#include "input/input_listener.hpp"

namespace tz
{
	template<tz::input::KeyPressCallback T, typename... Args>
	tz::input::KeyListener&	IWindow::emplace_custom_key_listener(T callback, Args&&... args)
	{
		auto custom_listener_ptr = std::make_unique<tz::input::CustomKeyListener<T>>(callback, std::forward<Args>(args)...);
		tz::input::KeyListener* ptr = custom_listener_ptr.get();
		this->register_key_listener(std::move(custom_listener_ptr));
		return *ptr;
	}
	
	template<tz::input::CharPressCallback T, typename... Args>
	tz::input::TypeListener& IWindow::emplace_custom_type_listener(T callback, Args&&... args)
	{
		auto custom_listener_ptr = std::make_unique<tz::input::CustomTypeListener<T>>(callback, std::forward<Args>(args)...);
		tz::input::TypeListener* ptr = custom_listener_ptr.get();
		this->register_type_listener(std::move(custom_listener_ptr));
		return *ptr;
	}

	template<tz::input::MouseUpdateCallback TUpdate, tz::input::MouseClickCallback TClick, typename... Args>
	tz::input::MouseListener& IWindow::emplace_custom_mouse_listener(TUpdate update, TClick click, Args&&... args)
	{
		auto custom_listener_ptr = std::make_unique<tz::input::CustomMouseListener<TUpdate, TClick>>(update, click, std::forward<Args>(args)...);
		tz::input::MouseListener* ptr = custom_listener_ptr.get();
		this->register_mouse_listener(std::move(custom_listener_ptr));
		return *ptr;
	}
}