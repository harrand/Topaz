#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include <iostream>

std::shared_ptr<World> world;
std::vector<std::shared_ptr<Mesh>> allMeshes;
std::vector<std::shared_ptr<Texture>> allTextures;
Camera cam(Vector3F(), Vector3F(0, 3.14159, 0));

void handleKeybinds(Window& wnd, KeyListener kl, float avgFrameMillis)
{
	//float multiplier = MathsUtility::parseTemplate(FileUtility::getTag(File(RES_POINT + "/resources.data"), "speed")) * (avgFrameMillis/1000);
	float multiplier = MathsUtility::parseTemplate(MDLF(RawFile(RES_POINT + "/resources.data")).getTag("speed")) * (avgFrameMillis / 1000);
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
}

// SDL2 defines main.
#ifdef main
#undef main
#endif
int main()
{	
	std::cout << "== Ocular GEng Development Testing ==\n";
	
	MDLF timeStorage(RawFile(RES_POINT + "/resources.data"));
	//int secondsLifetime = MathsUtility::parseTemplate(FileUtility::getTag(timeStorage, "played"));
	int secondsLifetime = MathsUtility::parseTemplate(timeStorage.getTag("played"));
	KeyListener kl;
	
	Window wnd(800, 600, "Ocular Game Engine : Test Window");
	std::cout << "GLSL Version = " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
	wnd.registerListener(kl);
	TimeKeeper tk, fpscounter;
	std::cout << "'Ocular GEng Development Window' created.\n";
	
	Shader shader(RES_POINT + "/shaders/vanilla");
	std::cout << "Ocular GEng Shader 'vanilla' initialised.\n";
	
	world = std::shared_ptr<World>(new World(RES_POINT + "/data/worlds/test.world"));
	//world = new World("./res/data/worlds/test.world");
	
	DataTranslation dt(RES_POINT + "/resources.data");
	typedef std::map<std::string, std::string>::iterator iter;
	std::map<std::string, std::string> models = dt.retrieveModels(), textures = dt.retrieveTextures();
	
	typedef std::map<std::string, std::string>::iterator it_type;
	std::cout << "Retriving models...\n";
	for(it_type iterator = models.begin(); iterator != models.end(); iterator++)
	{
		std::cout << "Initialising a mesh with the link " << iterator->first << ".\n";
		allMeshes.push_back(std::shared_ptr<Mesh>(new Mesh(iterator->first)));
	}
	std::cout << "Retriving textures...\n";
	for(it_type iterator = textures.begin(); iterator != textures.end(); iterator++)
	{
		std::cout << "Initialising a texture with the link " << iterator->first << ".\n";
		allTextures.push_back(std::shared_ptr<Texture>(new Texture(iterator->first)));
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
		
		handleKeybinds(wnd, kl, deltaTotal / deltas.size());
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
	//FileUtility::setTag(timeStorage, "played", strum.str());
	timeStorage.deleteTag("played");
	timeStorage.addTag("played", strum.str());
	return 0;
}