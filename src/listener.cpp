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

MouseController::MouseController(Camera& camera, World& world): camera(camera), world(world), ml(){}

MouseController::MouseController(const MouseController& copy): MouseController(copy.camera, copy.world){}

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
		Vector3F& orientation = this->camera.getRotationR();
		Vector2F delta = this->ml.getMouseDeltaPos();
		orientation.getYR() += (3 * delta.getX());
		orientation.getXR() -= (3 * delta.getY());
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

KeybindController::KeybindController(Camera& camera, const Shader& shader, World& world): camera(camera), shader(shader), world(world){}

KeybindController::KeybindController(const KeybindController& copy): KeybindController(copy.camera, copy.shader, copy.world){}

const KeyListener& KeybindController::getKeyListener() const
{
	return this->kl;
}

KeyListener& KeybindController::getKeyListenerR()
{
	return this->kl;
}

void KeybindController::handleKeybinds(float seconds_since_last_frame, std::string resources_path, std::string controls_path)
{
	float multiplier = tz::util::cast::fromString<float>(MDLF(RawFile(resources_path)).getTag("speed"));
	MDLF controls_data_file = MDLF(RawFile(controls_path));
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_FORWARD)))
		this->camera.getPositionR() += (this->camera.getForward() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_BACKWARD)))
		this->camera.getPositionR() += (this->camera.getBackward() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_LEFT)))
		this->camera.getPositionR() += (this->camera.getLeft() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_RIGHT)))
		this->camera.getPositionR() += (this->camera.getRight() * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_UP)))
		this->camera.getPositionR() += (Vector3F(0, 1, 0) * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::MOVE_DOWN)))
		this->camera.getPositionR() += (Vector3F(0, -1, 0) * multiplier * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_UP)))
		this->camera.getRotationR() += (Vector3F(1.0f/360.0f, 0, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_DOWN)))
		this->camera.getRotationR() += (Vector3F(-1.0f/360.0f, 0, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_LEFT)))
		this->camera.getRotationR() += (Vector3F(0, -1.0f/360.0f, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::LOOK_RIGHT)))
		this->camera.getRotationR() += (Vector3F(0, 1.0f/360.0f, 0) * multiplier * 5 * seconds_since_last_frame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controls_data_file, KeybindType::RESTART)))
	{
		camera.getPositionR() = this->world.getSpawnPoint();
		camera.getRotationR() = this->world.getSpawnOrientation();
	}
}
