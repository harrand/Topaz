#include "listener.hpp"
#include <algorithm>

// Static objects need to be initialised like this
unsigned int Listener::number_of_listeners = 0;

Listener::Listener(): id(Listener::number_of_listeners++){}

Listener::~Listener()
{
	Listener::number_of_listeners--;
}

unsigned int Listener::getID() const
{
	return this->id;
}

// static
unsigned int Listener::getNumListeners()
{
	return Listener::number_of_listeners;
}

MouseListener::MouseListener(): Listener(){}

void MouseListener::handleEvents(SDL_Event& evt)
{
	this->reloadMouseDelta();
	if(evt.type == SDL_MOUSEMOTION)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		this->mouse_position.getXR() = x;
		this->mouse_position.getYR() = y;
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

void MouseListener::reloadMouseDelta()
{
	this->previous_mouse_position = this->mouse_position;
}

bool MouseListener::isLeftClicked() const
{
	return this->left_click;
}

bool MouseListener::isRightClicked() const
{
	return this->right_click;
}

const Vector2F& MouseListener::getMousePos() const
{
	return this->mouse_position;
}

Vector2F MouseListener::getMouseDeltaPos() const
{
	return (this->mouse_position - this->previous_mouse_position);
}

const Vector2F& MouseListener::getLeftClickLocation() const
{
	return this->left_click_location;
}

const Vector2F& MouseListener::getRightClickLocation() const
{
	return this->right_click_location;
}

KeyListener::KeyListener(): Listener(){}

void KeyListener::handleEvents(SDL_Event& evt)
{
	switch(evt.type)
	{
		case SDL_KEYDOWN:
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

bool KeyListener::isKeyPressed(const std::string& keyname) const
{
	return (std::find(this->pressed_keys.begin(), this->pressed_keys.end(), keyname) != this->pressed_keys.end());
}

bool KeyListener::isKeyReleased(const std::string& keyname) const
{
	return (std::find(this->released_keys.begin(), this->released_keys.end(), keyname) != this->released_keys.end());
}

bool KeyListener::catchKeyPressed(const std::string& keyname)
{
	bool pressed = this->isKeyPressed(keyname);
	if(pressed)
		this->pressed_keys.erase(std::remove(this->pressed_keys.begin(), this->pressed_keys.end(), keyname), this->pressed_keys.end());
	return pressed;
}

bool KeyListener::catchKeyReleased(const std::string& keyname)
{
	bool released = this->isKeyReleased(keyname);
	if(released)
		this->released_keys.erase(std::remove(this->released_keys.begin(), this->released_keys.end(), keyname), this->released_keys.end());
	return released;
}
