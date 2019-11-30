#include "core/listener.hpp"
#include "core/window.hpp"

Listener::Listener(): window(nullptr){}

Listener::Listener(Window& window): window(&window)
{
	this->window->register_listener(*this);
}

Listener::~Listener()
{
	if(this->window != nullptr)
		this->window->register_listener(*this);
}

MouseListener::MouseListener(): Listener(), left_click(false), right_click(false), left_click_location(), right_click_location(), previous_mouse_position(), mouse_position(){}

MouseListener::MouseListener(Window& window): Listener(window), left_click(false), right_click(false), left_click_location(), right_click_location(), previous_mouse_position(), mouse_position(){}

MouseListener::MouseListener(const MouseListener& copy): MouseListener()
{
	if(copy.window != nullptr)
	{
		copy.window->register_listener(*this);
		this->window = copy.window;
	}
}

MouseListener::MouseListener(MouseListener&& move): MouseListener()
{
	Window* wnd = move.window;
	if(wnd != nullptr)
	{
		wnd->deregister_listener(move);
		wnd->register_listener(*this);
		this->window = wnd;
	}
}

void MouseListener::handle_events(const SDL_Event& evt)
{
	topaz_assert(this->window != nullptr, "MouseListener::handle_events(...): Invoked without an attached window.");
	this->reload_mouse_delta();
	if(evt.type == SDL_MOUSEMOTION)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		this->mouse_position.x = x;
		this->mouse_position.y = this->window->get_height() - y;
	}
	if(evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if(evt.button.button == SDL_BUTTON_LEFT)
		{
			this->left_click = true;
			this->left_click_location = this->mouse_position;
		}
		else if(evt.button.button == SDL_BUTTON_RIGHT)
		{
			this->right_click = true;
			this->right_click_location = this->mouse_position;
		}
	}
		
	if(evt.type == SDL_MOUSEBUTTONUP)
	{
		if(evt.button.button == SDL_BUTTON_LEFT)
			this->left_click = false;
		else if(evt.button.button == SDL_BUTTON_RIGHT)
			this->right_click = false;
	}
}

void MouseListener::reload_mouse_delta()
{
	this->previous_mouse_position = this->mouse_position;
}

bool MouseListener::is_left_clicked() const
{
	return this->left_click;
}

bool MouseListener::is_right_clicked() const
{
	return this->right_click;
}

const Vector2F& MouseListener::get_mouse_position() const
{
	return this->mouse_position;
}

Vector2F MouseListener::get_mouse_delta_position() const
{
	return (this->mouse_position - this->previous_mouse_position);
}

const Vector2F& MouseListener::get_left_click_location() const
{
	return this->left_click_location;
}

const Vector2F& MouseListener::get_right_click_location() const
{
	return this->right_click_location;
}

KeyListener::KeyListener(): Listener(){}

KeyListener::KeyListener(Window& window) : KeyListener()
{
	window.register_listener(*this);
}

KeyListener::KeyListener(const KeyListener& copy): KeyListener()
{
	if(copy.window != nullptr)
	{
		copy.window->register_listener(*this);
		this->window = copy.window;
	}
}

KeyListener::KeyListener(KeyListener&& move): KeyListener()
{
	Window* wnd = move.window;
	if(wnd != nullptr)
	{
		wnd->deregister_listener(move);
		wnd->register_listener(*this);
		this->window = move.window;
	}
}

void KeyListener::handle_events(const SDL_Event& evt)
{
	using namespace tz::utility;
	std::string key_name = {SDL_GetKeyName(evt.key.keysym.sym)};
	switch(evt.type)
	{
		case SDL_KEYDOWN:
			this->on_key_pressed(key_name);
			if(generic::contains(this->released_keys, key_name))
			{
				this->released_keys.erase(std::remove(this->released_keys.begin(), this->released_keys.end(), key_name), this->released_keys.end());
			}
			if(!generic::contains(this->pressed_keys, key_name))
			{
				// doesnt yet contain it.
				this->pressed_keys.push_back(key_name);
			}
		break;
		case SDL_KEYUP:
			this->on_key_released(key_name);
			if(!generic::contains(this->released_keys, key_name))
			{
				this->released_keys.push_back(key_name);
			}
			if(generic::contains(this->pressed_keys, key_name))
			{
				// does actually contain it
				this->pressed_keys.erase(std::remove(this->pressed_keys.begin(), this->pressed_keys.end(), key_name), this->pressed_keys.end());
			}
		break;
	}
}

bool KeyListener::is_key_pressed(const std::string& keyname) const
{
	return tz::utility::generic::contains(this->pressed_keys, keyname);
}

bool KeyListener::is_key_released(const std::string& keyname) const
{
	return tz::utility::generic::contains(this->released_keys, keyname);
}

bool KeyListener::catch_key_pressed(const std::string& keyname)
{
	bool pressed = this->is_key_pressed(keyname);
	if(pressed)
		this->pressed_keys.erase(std::remove(this->pressed_keys.begin(), this->pressed_keys.end(), keyname), this->pressed_keys.end());
	return pressed;
}

bool KeyListener::catch_key_released(const std::string& keyname)
{
	bool released = this->is_key_released(keyname);
	if(released)
		this->released_keys.erase(std::remove(this->released_keys.begin(), this->released_keys.end(), keyname), this->released_keys.end());
	return released;
}

namespace tz::listener
{
	bool is_mouse(const Listener* listener)
	{
		return tz::utility::functional::is_a<const Listener, const MouseListener>(*listener);
	}
	
	bool is_keyboard(const Listener* listener)
	{
		return tz::utility::functional::is_a<const Listener, const KeyListener>(*listener);
	}
}