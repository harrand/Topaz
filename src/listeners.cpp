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

KeybindType KeyControls::getKeybindType(std::string keyBindType)
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
		default:
			return "0";
		break;
	}
}

KeybindController::KeybindController(Camera& cam, std::shared_ptr<World>& world, Window& wnd): cam(cam), world(world), wnd(wnd)
{
	wnd.registerListener(this->kl);
}

KeybindController::~KeybindController()
{
	wnd.deregisterListener(this->kl);
}

void KeybindController::handleKeybinds(float avgFrameMillis)
{
	float multiplier = MathsUtility::parseTemplate(MDLF(RawFile(RES_POINT + "/resources.data")).getTag("speed")) * (avgFrameMillis / 1000);
	MDLF controlsDataFile = MDLF(RawFile(RES_POINT + "/controls.data"));
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_FORWARD)))
		this->cam.getPosR() += (cam.getForward() * multiplier);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_BACKWARD)))
		this->cam.getPosR() += (cam.getBackward() * multiplier);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_LEFT)))
		this->cam.getPosR() += (cam.getLeft() * multiplier);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_RIGHT)))
		this->cam.getPosR() += (cam.getRight() * multiplier);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_UP)))
		this->cam.getPosR() += (Vector3F(0, 1, 0) * multiplier);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::MOVE_DOWN)))
		this->cam.getPosR() += (Vector3F(0, -1, 0) * multiplier);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_UP)))
		cam.getRotR() += Vector3F(0.05, 0, 0);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_DOWN)))
		cam.getRotR() += Vector3F(-0.05, 0, 0);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_LEFT)))
		cam.getRotR() += Vector3F(0, -0.05, 0);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::LOOK_RIGHT)))
		cam.getRotR() += Vector3F(0, 0.05, 0);
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::INPUT_COMMAND)))
	{
		std::string input;
		std::getline(std::cin, input);
		Commands::inputCommand(input, world, cam);
	}
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::REQUEST_CLOSE)))
		wnd.requestClose();
	if(kl.isKeyPressed(KeyControls::getKeybind(controlsDataFile, KeybindType::RESTART)))
	{
		cam.getPosR() = Vector3F(0, 0, 0);
		cam.getRotR() = Vector3F(0, 3.14159, 0);
	}
	/*
	if(kl.isKeyPressed("W"))
	{
		cam.getPosR() += (cam.getForward() * multiplier);
	}
	if(kl.isKeyPressed("A"))
	{
		cam.getPosR() += (cam.getLeft() * multiplier);
	}
	if(kl.isKeyPressed("S"))
	{
		cam.getPosR() += (cam.getBackward() * multiplier);
	}
	if(kl.isKeyPressed("D"))
	{
		cam.getPosR() += (cam.getRight() * multiplier);
	}
	if(kl.isKeyPressed("Space"))
	{
		cam.getPosR() +=  (Vector3F(0, 1, 0) * multiplier);
	}
	if(kl.isKeyPressed("Z"))
	{
		cam.getPosR() += (Vector3F(0, -1, 0) * multiplier);
	}
	if(kl.isKeyPressed("R"))
	{
		cam.getPosR() = Vector3F(0, 0, 0);
		cam.getRotR() = Vector3F(0, 3.14159, 0);
		std::cout << "[DEV]: Teleported to [0, 0, 0], reoriented to [0, pi, 0].\n";
	}
	if(kl.isKeyPressed("Escape"))
	{
		wnd.requestClose();
	}
	if(kl.isKeyPressed("I"))
	{
		cam.getRotR() += Vector3F(0.05, 0, 0);
	}
	if(kl.isKeyPressed("K"))
	{
		cam.getRotR() += Vector3F(-0.05, 0, 0);
	}
	if(kl.isKeyPressed("J"))
	{
		cam.getRotR() += Vector3F(0, -0.05, 0);
	}
	if(kl.isKeyPressed("L"))
	{
		cam.getRotR() += Vector3F(0, 0.05, 0);
	}
	if(kl.isKeyPressed("Tab"))
	{
		std::string input;
		std::getline(std::cin, input);
		Commands::inputCommand(input, world, cam);
	}
	*/
}
