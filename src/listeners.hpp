#ifndef LISTENERS_HPP
#define LISTENERS_HPP

#include "window.hpp"
#include "command.hpp"
#include <vector>
#include <string>
#include <algorithm>

class KeyListener: public Listener
{
public:
	KeyListener(): Listener(){}
	~KeyListener(){}
	void handleEvents(SDL_Event& evt);
	//void reload();
	bool isKeyPressed(std::string keyname);
	bool isKeyReleased(std::string keyname);
	bool catchKeyPressed(std::string keyname);
	bool catchKeyReleased(std::string keyname);
private:
	std::vector<std::string> pressedKeys;
	std::vector<std::string> releasedKeys;
};

enum class KeybindType : unsigned int
{
	MOVE_FORWARD = 0,
	MOVE_BACKWARD = 1,
	MOVE_LEFT = 2,
	MOVE_RIGHT = 3,
	MOVE_UP = 4,
	MOVE_DOWN = 5,
	LOOK_UP = 6,
	LOOK_DOWN = 7,
	LOOK_LEFT = 8,
	LOOK_RIGHT = 9,
	INPUT_COMMAND = 10,
	REQUEST_CLOSE = 11,
	RESTART = 12,
	ADDDEFAULTOBJECT = 13,
	NIL = 14
};

namespace KeyControls
{
	KeybindType getKeybindType(std::string keyBindType);
	std::string getKeybind(MDLF& controlsDataFile, KeybindType kt);
}

class KeybindController
{
public:
	KeybindController(Camera& cam, std::shared_ptr<World>& world, Window& wnd);
	~KeybindController();
	void handleKeybinds(float avgFrameMillis);
	//void reload();
private:
	Camera& cam;
	std::shared_ptr<World>& world;
	Window& wnd;
	float avgFrameMillis;
	KeyListener kl;
};

#endif