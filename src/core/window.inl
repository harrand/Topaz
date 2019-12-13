#include "input/input_listener.hpp"

namespace tz::core
{
	template<typename T, typename... Args>
	tz::input::KeyListener&	IWindow::emplace_custom_key_listener(T callback, Args&&... args)
	{
		auto custom_listener_ptr = std::make_shared<tz::input::CustomKeyListener<T>>(callback, std::forward<Args>(args)...);
		this->register_key_listener(custom_listener_ptr);
		return *custom_listener_ptr;
	}
	
	template<typename T, typename... Args>
	tz::input::TypeListener& IWindow::emplace_custom_type_listener(T callback, Args&&... args)
	{
		auto custom_listener_ptr = std::make_shared<tz::input::CustomTypeListener<T>>(callback, std::forward<Args>(args)...);
		this->register_type_listener(custom_listener_ptr);
		return *custom_listener_ptr;
	}
}