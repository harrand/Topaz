#ifndef LISTENER_HPP
#define LISTENER_HPP
#include "SDL2/SDL.h"
#include "command.hpp"
#include "camera.hpp"
#include "scene.hpp"

/**
* Wrapper for an SDL_Event listener.
* Abstract. Not available for non-polymorphic use.
* Inherit from this to create custom listeners that a Topaz Window can register.
*/
class Listener
{
public:
	Listener();
	Listener(const Listener& copy) = delete;
	Listener(Listener&& move) = delete;
	Listener& operator=(const Listener& rhs) = delete;
	~Listener();
	
	virtual void handle_events(SDL_Event& evt) = 0;
	unsigned int get_id() const;
	static unsigned int get_num_listeners();
private:
	static unsigned int number_of_listeners;
	unsigned int id;
};

/**
* How Topaz handles mouse input.
* Register this to a Topaz Window to use properly.
*/
class MouseListener: public Listener
{
public:
	MouseListener();
	MouseListener(const MouseListener& copy) = default;
	MouseListener(MouseListener&& move) = default;
	~MouseListener() = default;
	MouseListener& operator=(const MouseListener& rhs) = default;
	
	virtual void handle_events(SDL_Event& evt) override;
	void reload_mouse_delta();
	bool is_left_clicked() const;
	bool is_right_clicked() const;
	const Vector2F& get_mouse_pos() const;
	Vector2F get_mouse_delta_pos() const;
	const Vector2F& get_left_click_location() const;
	const Vector2F& get_right_click_location() const;
private:
	bool left_click, right_click;
	Vector2F left_click_location, right_click_location;
	Vector2F previous_mouse_position, mouse_position;
};

/**
* How Topaz handles keyboard input.
* Register this to a Topaz Window to use properly.
*/
class KeyListener: public Listener
{
public:
	KeyListener();
	KeyListener(const KeyListener& copy) = default;
	KeyListener(KeyListener&& move) = default;
	~KeyListener() = default;
	KeyListener& operator=(const KeyListener& rhs) = default;
	
	virtual void handle_events(SDL_Event& evt) override;
	bool is_key_pressed(const std::string& keyname) const;
	bool is_key_released(const std::string& keyname) const;
	bool catch_key_pressed(const std::string& keyname);
	bool catch_key_released(const std::string& keyname);
private:
	std::vector<std::string> pressed_keys;
	std::vector<std::string> released_keys;
};

namespace tz::listener
{
	bool is_mouse(const Listener* listener);
	bool is_keyboard(const Listener* listener);
}

#endif