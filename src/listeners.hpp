#ifndef LISTENERS_HPP
#define LISTENERS_HPP
#include "window.hpp"
#include "command.hpp"

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

class MouseController
{
public:
	MouseController(Player& player, World& world, Window& wnd);
	MouseController(const MouseController& copy);
	MouseController(MouseController&& move) = delete;
	MouseController& operator=(const MouseController& rhs) = delete;
	~MouseController();
	const MouseListener& getMouseListener();
	MouseListener& getMouseListenerR();
	virtual void handleMouse();
private: 
	Player& player;
	World& world;
	Window& wnd;
	MouseListener ml;
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
	TOGGLE_FULLSCREEN = 10,
	INPUT_COMMAND = 11,
	REQUEST_CLOSE = 12,
	RESTART = 13,
	ALIAS = 14,
	NIL = 15
};

namespace KeyControls
{
	KeybindType getKeybindType(const std::string& keybind_type);
	std::string getKeybind(MDLF& controls_data_file, KeybindType kt);
}

class KeybindController
{
public:
	KeybindController(Player& player, const Shader& shader, World& world, Window& wnd);
	KeybindController(const KeybindController& copy);
	KeybindController(KeybindController&& move) = delete;
	KeybindController& operator=(const KeybindController& rhs) = delete;
	~KeybindController();
	void handleKeybinds(float seconds_since_last_frame, std::string resources_path, std::string controls_path);
private:
	Player& player;
	const Shader& shader;
	World& world;
	Window& wnd;
	KeyListener kl;
};

#endif