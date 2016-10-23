#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include <iostream>

World* world;
std::vector<Mesh*> allMeshes;
std::vector<Texture*> allTextures;
Camera cam(Vector3F(), Vector3F(0, 3.14159, 0));

void handleCmd(std::string input)
{
	std::vector<std::string> args;
	if(StringUtility::contains(input, ' '))
		args = StringUtility::splitString(input, ' ');
	else
		args.push_back(input);
	std::string n = args.at(0);
	if(n == "loadworld")
		Commands::loadWorld(args, world);
	else if(n == "exportworld")
		Commands::exportWorld(args, world);
	else if(n == "addobject")
		Commands::addObject(args, world, cam, true);
	else if(n == "reloadworld")
		Commands::reloadWorld(world, true);
	else
		std::cout << "Unknown command '" << n << "'.\n";
}

void handleKeybinds(Window* wnd, KeyListener kl, float avgFrameMillis)
{
	float multiplier = MathsUtility::parseTemplate(FileUtility::getTag(File("./res/resources.data"), "speed")) * (avgFrameMillis/1000);
	
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
		wnd->requestClose();
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
		handleCmd(input);
	}
}

// SDL2 defines main.
#ifdef main
#undef main
#endif
int main()
{	
	std::cout << "== Ocular GEng Development Testing ==\n";
	
	File timeStorage("./res/resources.data");
	int secondsLifetime = MathsUtility::parseTemplate(FileUtility::getTag(timeStorage, "played"));
	KeyListener kl;
	
	Window wnd(800, 600, "Ocular Game Engine : Test Window");
	std::cout << "GLSL Version = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
	wnd.registerListener(kl);
	TimeKeeper tk, fpscounter;
	std::cout << "'Ocular GEng Development Window' created.\n";
	
	Shader shader("./res/shaders/vanilla");
	std::cout << "Ocular GEng Shader 'vanilla' initialised.\n";
	
	world = new World("./res/data/worlds/test.world");
	
	DataTranslation dt("./res/resources.data");
	typedef std::map<std::string, std::string>::iterator iter;
	std::map<std::string, std::string> models = dt.retrieveModels(), textures = dt.retrieveTextures();
	
	typedef std::map<std::string, std::string>::iterator it_type;
	for(it_type iterator = models.begin(); iterator != models.end(); iterator++)
	{
		// iterator->first = key = model path
		// iterator->second = value = model 'name'
		//std::cout << "Model Detected of name '" << iterator->second << "' and is at the path '" << iterator->first << "'.\n";
		allMeshes.push_back(new Mesh(iterator->first));
	}
	for(it_type iterator = textures.begin(); iterator != textures.end(); iterator++)
	{
		// iterator->first = key = model path
		// iterator->second = value = model 'name'
		//std::cout << "Texture Detected of name '" << iterator->second << "' and is at the path '" << iterator->first << "'.\n";
		allTextures.push_back(new Texture(iterator->first));
	}
	
	std::vector<float> deltas;
	float deltaTotal = 0.0f, deltaAverage = 0.0f;
	unsigned long fps;
	
	std::cout << "\nOcular GEng Window Loop Began:\n";
	while(!wnd.isCloseRequested())
	{
		if(tk.millisPassed(1000))
		{
			deltaAverage = deltaTotal/deltas.size();
			fps = round(1000 / deltaAverage);
			std::cout << "avgdt = " << deltaAverage << "ms, avgFPS = " << fps << " frames per second.\n";
			
			deltaAverage = 0;
			deltaTotal = 0;
			deltas.clear();
			secondsLifetime++;
			tk.reload();
			std::cout << "Camera Position = [" << cam.getPosR().getX() << ", " << cam.getPosR().getY() << ", " << cam.getPosR().getZ() << "].\n";
			std::cout << "Lifetime Spent: " << secondsLifetime << " seconds.\n";
			//std::cout << "Current World Size = " << world->getSize() << ".\n";
		}
		fpscounter.update();
		deltaTotal += fpscounter.getRange();
		deltas.push_back(fpscounter.getRange());
		
		tk.update();
		wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
		shader.bind();
		
		handleKeybinds(&wnd, kl, deltaTotal / deltas.size());
		fpscounter.reload();
		
		for(unsigned int i = 0; i < world->getMembers().size(); i++)
		{
			Object obj = world->getMembers().at(i);
			obj.render(Mesh::getFromLink(obj.getMeshLink(), allMeshes), Texture::getFromLink(obj.getTextureLink(), allTextures), cam, shader, wnd.getWidth(), wnd.getHeight());
		}
		
		wnd.update();
		wnd.setTitle("Game - World '" + world->getWorldLink() + "'");
	}
	std::ostringstream strum;
	strum << secondsLifetime;
	FileUtility::setTag(timeStorage, "played", strum.str());
	
	for(unsigned int i = 0; i < allMeshes.size(); i++)
	{
		delete allMeshes.at(i);
	}
	for(unsigned int i = 0; i < allTextures.size(); i++)
	{
		delete allTextures.at(i);
	}
	
	delete world;
	return 0;
}