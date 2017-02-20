#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include <iostream>

std::shared_ptr<World> world;
std::vector<std::shared_ptr<Mesh>> allMeshes;
std::vector<std::shared_ptr<Texture>> allTextures;
Camera cam(Vector3F(), Vector3F(0, 3.14159, 0));

// SDL2 defines main.
#ifdef main
#undef main
#endif
int main()
{	
	MDLF timeStorage(RawFile(RES_POINT + "/resources.data"));
	int secondsLifetime = CastUtility::fromString<int>(timeStorage.getTag("played"));
	
	Window wnd(800, 600, "Ocular Game Engine : Test Window");	
	TimeKeeper tk, fpscounter;
	
	std::cout << "'Ocular GEng Testing Environment created.\n";
	
	Shader shader(RES_POINT + "/shaders/vanilla");	
	world = std::shared_ptr<World>(new World(RES_POINT + "/data/worlds/test.world"));
	KeybindController kc(cam, world, wnd);
	DataTranslation dt(RES_POINT + "/resources.data");
	
	std::map<std::string, std::string> models = dt.retrieveModels(), textures = dt.retrieveTextures();
	
	typedef std::map<std::string, std::string>::iterator it_type;
	std::cout << "Retrieving models...\n";
	for(it_type iterator = models.begin(); iterator != models.end(); iterator++)
	{
		std::cout << "Initialising a mesh with the link " << iterator->first << ".\n";
		allMeshes.push_back(std::shared_ptr<Mesh>(new Mesh(iterator->first)));
	}
	std::cout << "Retrieving textures...\n";
	for(it_type iterator = textures.begin(); iterator != textures.end(); iterator++)
	{
		std::cout << "Initialising a texture with the link " << iterator->first << ".\n";
		allTextures.push_back(std::shared_ptr<Texture>(new Texture(iterator->first)));
	}
	std::vector<float> deltas;
	float deltaTotal = 0.0f, deltaAverage = 0.0f;
	unsigned long fps;

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
		}
		fpscounter.update();
		deltaTotal += fpscounter.getRange();
		deltas.push_back(fpscounter.getRange());
		
		tk.update();
		wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
		shader.bind();
		
		kc.handleKeybinds(deltaTotal / deltas.size());
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
	timeStorage.deleteTag("played");
	timeStorage.addTag("played", strum.str());
	return 0;
}