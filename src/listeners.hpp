#ifndef LISTENERS_HPP
#define LISTENERS_HPP

#include "window.hpp"
#include <vector>
#include <string>
#include <algorithm>

class KeyListener: public Listener
{
public:
	KeyListener(): Listener(){}
	~KeyListener(){}
	void handleEvents(SDL_Event& evt);
	bool isKeyPressed(std::string keyname);
private:
	std::vector<std::string> pressedKeys;
};

#endif