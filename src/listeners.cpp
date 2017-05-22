#include "listeners.hpp"

MouseListener::MouseListener(): Listener(){}

void MouseListener::handleEvents(SDL_Event& evt)
{
	this->reloadMouseDelta();
	if(evt.type == SDL_MOUSEMOTION)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		this->mousePos.getXR() = x;
		this->mousePos.getYR() = y;
	}
	if(evt.type == SDL_MOUSEBUTTONDOWN)
	{
		if(evt.button.button == SDL_BUTTON_LEFT)
			this->leftClick = true;
		else if(evt.button.button == SDL_BUTTON_RIGHT)
			this->rightClick = true;
	}
		
	if(evt.type == SDL_MOUSEBUTTONUP)
	{
		if(evt.button.button == SDL_BUTTON_LEFT)
			this->leftClick = false;
		else if(evt.button.button == SDL_BUTTON_RIGHT)
			this->rightClick = false;
	}
}

void MouseListener::reloadMouseDelta()
{
	this->prevMousePos = this->mousePos;
}

bool MouseListener::isLeftClicked() const
{
	return this->leftClick;
}

bool MouseListener::isRightClicked() const
{
	return this->rightClick;
}

const Vector2F& MouseListener::getMousePos() const
{
	return this->mousePos;
}

Vector2F MouseListener::getMouseDeltaPos() const
{
	return (this->mousePos - this->prevMousePos);
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
		Vector3F& orientation = this->player.getCamera().getRotR();
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
			if(std::find(this->releasedKeys.begin(), this->releasedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)) != this->releasedKeys.end())
			{
				this->releasedKeys.erase(std::remove(this->releasedKeys.begin(), this->releasedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)), this->releasedKeys.end());
			}
			if(std::find(pressedKeys.begin(), pressedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)) == pressedKeys.end())
			{
				// doesnt yet contain it.
				this->pressedKeys.push_back(SDL_GetKeyName(evt.key.keysym.sym));
			}
		break;
		case SDL_KEYUP:
			if(std::find(this->releasedKeys.begin(), this->releasedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)) == this->releasedKeys.end())
			{
				this->releasedKeys.push_back(SDL_GetKeyName(evt.key.keysym.sym));
			}
			if(std::find(this->pressedKeys.begin(), this->pressedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)) != this->pressedKeys.end())
			{
				// does actually contain it
				this->pressedKeys.erase(std::remove(this->pressedKeys.begin(), this->pressedKeys.end(), SDL_GetKeyName(evt.key.keysym.sym)), this->pressedKeys.end());
			}
		break;
	}
}

bool KeyListener::isKeyPressed(const std::string& keyname) const
{
	return (std::find(this->pressedKeys.begin(), this->pressedKeys.end(), keyname) != this->pressedKeys.end());
}

bool KeyListener::isKeyReleased(const std::string& keyname) const
{
	return (std::find(this->releasedKeys.begin(), this->releasedKeys.end(), keyname) != this->releasedKeys.end());
}

bool KeyListener::catchKeyPressed(const std::string& keyname)
{
	bool pressed = this->isKeyPressed(keyname);
	if(pressed)
		this->pressedKeys.erase(std::remove(this->pressedKeys.begin(), this->pressedKeys.end(), keyname), this->pressedKeys.end());
	return pressed;
}

bool KeyListener::catchKeyReleased(const std::string& keyname)
{
	bool released = this->isKeyReleased(keyname);
	if(released)
		this->releasedKeys.erase(std::remove(this->releasedKeys.begin(), this->releasedKeys.end(), keyname), this->releasedKeys.end());
	return released;
}

KeybindType KeyControls::getKeybindType(const std::string& keyBindType)
{
	if(keyBindType == "MOVE_FORWARD")
		return KeybindType::MOVE_FORWARD;
	else if(keyBindType == "MOVE_BACKWARD")
		return KeybindType::MOVE_BACKWARD;
	else if(keyBindType == "MOVE_LEFT")
		return KeybindType::MOVE_LEFT;
	else if(keyBindType == "MOVE_RIGHT")
		return KeybindType::MOVE_RIGHT;
	else if(keyBindType == "MOVE_UP")
		return KeybindType::MOVE_UP;
	else if(keyBindType == "MOVE_DOWN")
		return KeybindType::MOVE_DOWN;
	else if(keyBindType == "LOOK_UP")
		return KeybindType::LOOK_UP;
	else if(keyBindType == "LOOK_DOWN")
		return KeybindType::LOOK_DOWN;
	else if(keyBindType == "LOOK_LEFT")
		return KeybindType::LOOK_LEFT;
	else if(keyBindType == "LOOK_RIGHT")
		return KeybindType::LOOK_RIGHT;
	else if(keyBindType == "INPUT_COMMAND")
		return KeybindType::INPUT_COMMAND;
	else if(keyBindType == "REQUEST_CLOSE")
		return KeybindType::REQUEST_CLOSE;
	else if(keyBindType == "RESTART")
		return KeybindType::RESTART;
	else if(keyBindType == "ALIAS")
		return KeybindType::ALIAS;
	else
		return KeybindType::NIL;
}

std::string KeyControls::getKeybind(MDLF& controlsDataFile, KeybindType kt)
{
	switch(kt)
	{
		case KeybindType::MOVE_FORWARD:
			return controlsDataFile.getTag("MOVE_FORWARD");
		break;
		case KeybindType::MOVE_BACKWARD:
			return controlsDataFile.getTag("MOVE_BACKWARD");
		break;
		case KeybindType::MOVE_LEFT:
			return controlsDataFile.getTag("MOVE_LEFT");
		break;
		case KeybindType::MOVE_RIGHT:
			return controlsDataFile.getTag("MOVE_RIGHT");
		break;
		case KeybindType::MOVE_UP:
			return controlsDataFile.getTag("MOVE_UP");
		break;
		case KeybindType::MOVE_DOWN:
			return controlsDataFile.getTag("MOVE_DOWN");
		break;
		case KeybindType::LOOK_UP:
			return controlsDataFile.getTag("LOOK_UP");
		break;
		case KeybindType::LOOK_DOWN:
			return controlsDataFile.getTag("LOOK_DOWN");
		break;
		case KeybindType::LOOK_LEFT:
			return controlsDataFile.getTag("LOOK_LEFT");
		break;
		case KeybindType::LOOK_RIGHT:
			return controlsDataFile.getTag("LOOK_RIGHT");
		break;
		case KeybindType::INPUT_COMMAND:
			return controlsDataFile.getTag("INPUT_COMMAND");
		break;
		case KeybindType::REQUEST_CLOSE:
			return controlsDataFile.getTag("REQUEST_CLOSE");
		break;
		case KeybindType::RESTART:
			return controlsDataFile.getTag("RESTART");
		break;
		case KeybindType::ALIAS:
			return controlsDataFile.getTag("ALIAS");
		break;
		default:
			return "0";
		break;
	}
}

KeybindController::KeybindController(Player& player, Shader& shader, World& world, Window& wnd): player(player), shader(shader), world(world), wnd(wnd)
{
	wnd.registerListener(this->kl);
}

KeybindController::KeybindController(const KeybindController& copy): KeybindController(copy.player, copy.shader, copy.world, copy.wnd){}

KeybindController::~KeybindController()
{
	wnd.deregisterListener(this->kl);
}

void KeybindController::handleKeybinds(float secondsSinceLastFrame)
{
	float multiplier = CastUtility::fromString<float>(MDLF(RawFile(RES_POINT + "/resources.data")).getTag("speed"));
	MDLF controlsDataFile = MDLF(RawFile(RES_POINT + "/controls.data"));
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_FORWARD)))
		this->player.getCamera().getPosR() += (player.getCamera().getForward() * multiplier * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_BACKWARD)))
		this->player.getCamera().getPosR() += (player.getCamera().getBackward() * multiplier * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_LEFT)))
		this->player.getCamera().getPosR() += (player.getCamera().getLeft() * multiplier * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_RIGHT)))
		this->player.getCamera().getPosR() += (player.getCamera().getRight() * multiplier * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_UP)))
		this->player.getCamera().getPosR() += (Vector3F(0, 1, 0) * multiplier * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_DOWN)))
		this->player.getCamera().getPosR() += (Vector3F(0, -1, 0) * multiplier * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_UP)))
		player.getCamera().getRotR() += (Vector3F(1.0f/360.0f, 0, 0) * multiplier * 5 * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_DOWN)))
		player.getCamera().getRotR() += (Vector3F(-1.0f/360.0f, 0, 0) * multiplier * 5 * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_LEFT)))
		player.getCamera().getRotR() += (Vector3F(0, -1.0f/360.0f, 0) * multiplier * 5 * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_RIGHT)))
		player.getCamera().getRotR() += (Vector3F(0, 1.0f/360.0f, 0) * multiplier * 5 * secondsSinceLastFrame);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::INPUT_COMMAND)))
	{
		std::string input;
		// dont bother with this, std::cin is not thread-safe
		//std::thread([&](){std::getline(std::cin, input); Commands::inputCommand(input, world, player, shader);}).detach();
		std::getline(std::cin, input);
		Commands::inputCommand(input, world, player, shader);
	}
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::REQUEST_CLOSE)))
		wnd.requestClose();
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::RESTART)))
	{
		player.getCamera().getPosR() = this->world.getSpawnPoint();
		player.getCamera().getRotR() = this->world.getSpawnOrientation();
		player.setVelocity(Vector3F());
	}
	if(kl.catchKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::ALIAS)))
	{
		std::vector<std::string> alias = CommandCache::getAlias();
		std::string cmd = "";
		for(size_t i = 0; i < alias.size(); i++)
			cmd += (i != (alias.size() - 1)) ? alias.at(i) + " " : alias.at(i);
		Commands::inputCommand(cmd, world, player, shader);
	}
}
