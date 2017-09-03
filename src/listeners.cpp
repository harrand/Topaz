#include "listeners.hpp"
#include <algorithm>


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
			this->left_click = true;
		else if(evt.button.button == SDL_BUTTON_RIGHT)
			this->right_click = true;
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

MouseController::MouseController(Player& player, World& world, Window& wnd): player(player), world(world), wnd(wnd), ml()
{
	this->wnd.registerListener(this->ml);
}

MouseController::MouseController(const MouseController& copy): MouseController(copy.player, copy.world, copy.wnd){}

MouseController::~MouseController()
{
	this->wnd.deregisterListener(this->ml);
}

const MouseListener& MouseController::getMouseListener()
{
	return this->ml;
}

MouseListener& MouseController::getMouseListenerR()
{
	return this->ml;
}

void MouseController::handleMouse()
{
	if(this->ml.isLeftClicked())
	{
		Vector3F& orientation = this->player.getCamera().getRotationR();
		Vector2F delta = this->ml.getMouseDeltaPos();
		orientation.getYR() += (3 * delta.getX() / (this->wnd.getWidth()));
		orientation.getXR() -= (3 * delta.getY() / (this->wnd.getHeight()));
	}
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

KeybindType KeyControls::getKeybindType(const std::string& keybind_type)
{
	if(keybind_type == "MOVE_FORWARD")
		return KeybindType::MOVE_FORWARD;
	else if(keybind_type == "MOVE_BACKWARD")
		return KeybindType::MOVE_BACKWARD;
	else if(keybind_type == "MOVE_LEFT")
		return KeybindType::MOVE_LEFT;
	else if(keybind_type == "MOVE_RIGHT")
		return KeybindType::MOVE_RIGHT;
	else if(keybind_type == "MOVE_UP")
		return KeybindType::MOVE_UP;
	else if(keybind_type == "MOVE_DOWN")
		return KeybindType::MOVE_DOWN;
	else if(keybind_type == "LOOK_UP")
		return KeybindType::LOOK_UP;
	else if(keybind_type == "LOOK_DOWN")
		return KeybindType::LOOK_DOWN;
	else if(keybind_type == "LOOK_LEFT")
		return KeybindType::LOOK_LEFT;
	else if(keybind_type == "LOOK_RIGHT")
		return KeybindType::LOOK_RIGHT;
	else if(keybind_type == "TOGGLE_FULLSCREEN")
		return KeybindType::TOGGLE_FULLSCREEN;
	else if(keybind_type == "INPUT_COMMAND")
		return KeybindType::INPUT_COMMAND;
	else if(keybind_type == "REQUEST_CLOSE")
		return KeybindType::REQUEST_CLOSE;
	else if(keybind_type == "RESTART")
		return KeybindType::RESTART;
	else if(keybind_type == "ALIAS")
		return KeybindType::ALIAS;
	else
		return KeybindType::NIL;
}

std::string KeyControls::getKeybind(MDLF& controls_data_file, KeybindType kt)
{
	switch(kt)
	{
		case KeybindType::MOVE_FORWARD:
			return controls_data_file.getTag("MOVE_FORWARD");
		break;
		case KeybindType::MOVE_BACKWARD:
			return controls_data_file.getTag("MOVE_BACKWARD");
		break;
		case KeybindType::MOVE_LEFT:
			return controls_data_file.getTag("MOVE_LEFT");
		break;
		case KeybindType::MOVE_RIGHT:
			return controls_data_file.getTag("MOVE_RIGHT");
		break;
		case KeybindType::MOVE_UP:
			return controls_data_file.getTag("MOVE_UP");
		break;
		case KeybindType::MOVE_DOWN:
			return controls_data_file.getTag("MOVE_DOWN");
		break;
		case KeybindType::LOOK_UP:
			return controls_data_file.getTag("LOOK_UP");
		break;
		case KeybindType::LOOK_DOWN:
			return controls_data_file.getTag("LOOK_DOWN");
		break;
		case KeybindType::LOOK_LEFT:
			return controls_data_file.getTag("LOOK_LEFT");
		break;
		case KeybindType::LOOK_RIGHT:
			return controls_data_file.getTag("LOOK_RIGHT");
		break;
		case KeybindType::TOGGLE_FULLSCREEN:
			return controls_data_file.getTag("TOGGLE_FULLSCREEN");
		break;
		case KeybindType::INPUT_COMMAND:
			return controls_data_file.getTag("INPUT_COMMAND");
		break;
		case KeybindType::REQUEST_CLOSE:
			return controls_data_file.getTag("REQUEST_CLOSE");
		break;
		case KeybindType::RESTART:
			return controls_data_file.getTag("RESTART");
		break;
		case KeybindType::ALIAS:
			return controls_data_file.getTag("ALIAS");
		break;
		default:
			return "0";
		break;
	}
}

KeybindController::KeybindController(Player& player, const Shader& shader, World& world, Window& wnd): player(player), shader(shader), world(world), wnd(wnd)
{
	wnd.registerListener(this->kl);
}

KeybindController::KeybindController(const KeybindController& copy): KeybindController(copy.player, copy.shader, copy.world, copy.wnd){}

KeybindController::~KeybindController()
{
	wnd.deregisterListener(this->kl);
}

void KeybindController::handleKeybinds(float seconds_since_last_frame, std::string resources_path, std::string controls_path)
{
	float multiplier = tz::util::cast::fromString<float>(MDLF(RawFile(resources_path)).getTag("speed"));
	MDLF controls_data_file = MDLF(RawFile(controls_path));
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_FORWARD)))
		this->player.getCamera().getPositionR() += (player.getCamera().getForward() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_BACKWARD)))
		this->player.getCamera().getPositionR() += (player.getCamera().getBackward() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_LEFT)))
		this->player.getCamera().getPositionR() += (player.getCamera().getLeft() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_RIGHT)))
		this->player.getCamera().getPositionR() += (player.getCamera().getRight() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_UP)))
		this->player.getCamera().getPositionR() += (Vector3F(0, 1, 0) * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_DOWN)))
		this->player.getCamera().getPositionR() += (Vector3F(0, -1, 0) * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_UP)))
		this->player.getCamera().getRotationR() += (Vector3F(1.0f/360.0f, 0, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_DOWN)))
		this->player.getCamera().getRotationR() += (Vector3F(-1.0f/360.0f, 0, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_LEFT)))
		this->player.getCamera().getRotationR() += (Vector3F(0, -1.0f/360.0f, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_RIGHT)))
		this->player.getCamera().getRotationR() += (Vector3F(0, 1.0f/360.0f, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::TOGGLE_FULLSCREEN)))
		SDL_SetWindowFullscreen(this->wnd.getWindowHandleR(), !(SDL_GetWindowFlags(this->wnd.getWindowHandleR()) & SDL_WINDOW_FULLSCREEN));
			
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::INPUT_COMMAND)))
	{
		std::string input;
		std::getline(std::cin, input);
		Commands::inputCommand(input, resources_path, world, player, shader);
	}
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::REQUEST_CLOSE)))
		wnd.requestClose();
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::RESTART)))
	{
		player.getCamera().getPositionR() = this->world.getSpawnPoint();
		player.getCamera().getRotationR() = this->world.getSpawnOrientation();
		player.setVelocity(Vector3F());
	}
	if(kl.catchKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::ALIAS)))
	{
		std::vector<std::string> alias = CommandCache::getAlias();
		std::string cmd = "";
		for(std::size_t i = 0; i < alias.size(); i++)
			cmd += (i != (alias.size() - 1)) ? alias.at(i) + " " : alias.at(i);
		Commands::inputCommand(cmd, resources_path, world, player, shader);
	}
}
