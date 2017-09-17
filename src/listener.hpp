#ifndef LISTENERS_HPP
#define LISTENERS_HPP
#include "SDL.h"
#include "command.hpp"
#include "camera.hpp"
#include "world.hpp"

class Listener
{
public:
	Listener();
	Listener(const Listener& copy) = delete;
	Listener(Listener&& move) = delete;
	Listener& operator=(const Listener& rhs) = delete;
	~Listener();
	
	virtual void handleEvents(SDL_Event& evt) = 0;
	
	unsigned int getID() const;
	
	static unsigned int getNumListeners();
private:
	static unsigned int number_of_listeners;
	unsigned int id;
};

class MouseListener: public Listener
{
public:
	MouseListener();
	MouseListener(const MouseListener& copy) = default;
	MouseListener(MouseListener&& move) = default;
	MouseListener& operator=(const MouseListener& rhs) = default;
	
	void handleEvents(SDL_Event& evt);
	
	void reloadMouseDelta();
	
	bool isLeftClicked() const;
	bool isRightClicked() const;
	
	const Vector2F& getMousePos() const;
	Vector2F getMouseDeltaPos() const;
private:
	bool left_click, right_click;
	Vector2F previous_mouse_position, mouse_position;
};

class KeyListener: public Listener
{
public:
	KeyListener();
	KeyListener(const KeyListener& copy) = default;
	KeyListener(KeyListener&& move) = default;
	KeyListener& operator=(const KeyListener& rhs) = default;
	
	virtual void handleEvents(SDL_Event& evt);
	
	bool isKeyPressed(const std::string& keyname) const;
	bool isKeyReleased(const std::string& keyname) const;
	bool catchKeyPressed(const std::string& keyname);
	bool catchKeyReleased(const std::string& keyname);
private:
	std::vector<std::string> pressed_keys;
	std::vector<std::string> released_keys;
};

#endif