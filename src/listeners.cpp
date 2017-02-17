#include "listeners.hpp"

void KeyListener::handleEvents(SDL_Event& evt)
{
	switch(evt.type)
	{
		case SDL_KEYDOWN:
			if(std::find(pressedKeys.begin(), pressedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)) == pressedKeys.end())
			{
				// doesnt yet contain it.
				pressedKeys.push_back(SDL_GetKeyName(evt.key.keysym.sym));
			}
		break;
		case SDL_KEYUP:
			if(std::find(pressedKeys.begin(), pressedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)) != pressedKeys.end())
			{
				// does actually contain it
				pressedKeys.erase(std::remove(pressedKeys.begin(), pressedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)), pressedKeys.end());
			}
		break;
	}
}

bool KeyListener::isKeyPressed(std::string keyname)
{
	return (std::find(pressedKeys.begin(), pressedKeys.end(), keyname) != pressedKeys.end());
}