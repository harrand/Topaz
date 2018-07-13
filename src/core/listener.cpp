#include "core/listener.hpp"

Listener::Listener(): window(nullptr){}

MouseListener::MouseListener(): Listener(), left_click(false), right_click(false), left_click_location(), right_click_location(), previous_mouse_position(), mouse_position(){}

MouseListener::MouseListener(Window& window): MouseListener()
{
	window.register_listener(*this);
}

void MouseListener::handle_events(const SDL_Event& evt)
{
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

void KeyListener::handle_events(const SDL_Event& evt)
{
	switch(evt.type)
	{
		case SDL_KEYDOWN:
			this->on_key_pressed(SDL_GetKeyName(evt.key.keysym.sym));
			if(std::find(this->released_keys.begin(), this->released_keys.end(), SDL_GetKeyName(evt.key.keysym.sym)) != this->released_keys.end())
			{
				this->released_keys.erase(std::remove(this->released_keys.begin(), this->released_keys.end(), SDL_GetKeyName(evt.key.keysym.sym)), this->released_keys.end());
			}
			if(std::find(pressed_keys.begin(), pressed_keys.end(), SDL_GetKeyName(evt.key.keysym.sym)) == pressed_keys.end())
			{
				// doesnt yet contain it.
				this->pressed_keys.push_back(SDL_GetKeyName(evt.key.keysym.sym));
			}
		break;
		case SDL_KEYUP:
			this->on_key_released(SDL_GetKeyName(evt.key.keysym.sym));
			if(std::find(this->released_keys.begin(), this->released_keys.end(), SDL_GetKeyName(evt.key.keysym.sym)) == this->released_keys.end())
			{
				this->released_keys.push_back(SDL_GetKeyName(evt.key.keysym.sym));
			}
			if(std::find(this->pressed_keys.begin(), this->pressed_keys.end(), SDL_GetKeyName(evt.key.keysym.sym)) != this->pressed_keys.end())
			{
				// does actually contain it
				this->pressed_keys.erase(std::remove(this->pressed_keys.begin(), this->pressed_keys.end(), SDL_GetKeyName(evt.key.keysym.sym)), this->pressed_keys.end());
			}
		break;
	}
}

bool KeyListener::is_key_pressed(const std::string& keyname) const
{
	return (std::find(this->pressed_keys.begin(), this->pressed_keys.end(), keyname) != this->pressed_keys.end());
}

bool KeyListener::is_key_released(const std::string& keyname) const
{
	return (std::find(this->released_keys.begin(), this->released_keys.end(), keyname) != this->released_keys.end());
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
		return dynamic_cast<const MouseListener*>(listener) != nullptr;
	}
	
	bool is_keyboard(const Listener* listener)
	{
		return dynamic_cast<const KeyListener*>(listener) != nullptr;
	}
}
