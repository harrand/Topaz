#ifndef LISTENERS_HPP
#define LISTENERS_HPP

#include "window.hpp"
#include "command.hpp"
#include <algorithm>

class MouseListener: public Listener
{
public:
	MouseListener(): Listener(){}
	~MouseListener(){}
	void handleEvents(SDL_Event& evt);
	void reloadMouseDelta();
	bool isLeftClicked() const;
	bool isRightClicked() const;
	const Vector2F& getMousePos() const;
	Vector2F getMouseDeltaPos() const;
private:
	bool leftClick, rightClick;
	Vector2F prevMousePos, mousePos;
};

class MouseController
{
public:
	MouseController(Player& player, std::shared_ptr<World>& world, Window& wnd);
	~MouseController();
	void handleMouse();
	MouseListener& getMouseListener();
private: 
	Player& player;
	std::shared_ptr<World>& world;
	Window& wnd;
	MouseListener ml;
};

class KeyListener: public Listener
{
public:
	KeyListener(): Listener(){}
	~KeyListener(){}
	void handleEvents(SDL_Event& evt);
	bool isKeyPressed(const std::string& keyname) const;
	bool isKeyReleased(const std::string& keyname) const;
	bool catchKeyPressed(const std::string& keyname);
	bool catchKeyReleased(const std::string& keyname);
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
	ALIAS = 13,
	NIL = 14
};

namespace KeyControls
{
	KeybindType getKeybindType(const std::string& keyBindType);
	std::string getKeybind(MDLF& controlsDataFile, KeybindType kt);
}

class KeybindController
{
public:
	KeybindController(Player& player, std::shared_ptr<World>& world, Window& wnd);
	~KeybindController();
	void handleKeybinds(unsigned int fps);
private:
	Player& player;
	std::shared_ptr<World>& world;
	Window& wnd;
	KeyListener kl;
};

#endif