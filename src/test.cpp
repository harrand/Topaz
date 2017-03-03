#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include <iostream>

std::shared_ptr<World> world;
std::vector<std::shared_ptr<Mesh>> allMeshes;
std::vector<std::shared_ptr<Texture>> allTextures;
Camera cam(Vector3F(), Vector3F(0, 3.14159, 0));
Player player(10, cam);

// SDL2 defines main.
#ifdef main
#undef main
#endif
int main()
{	
	MDLF timeStorage(RawFile(RES_POINT + "/resources.data"));
	int secondsLifetime = CastUtility::fromString<int>(timeStorage.getTag("played"));
	
	Window wnd(800, 600, "Topaz Game Engine : Test Window");
	TimeKeeper tk, fpscounter;
	
	std::cout << "'Topaz Testing Environment created.\n";
	
	Shader shader(RES_POINT + "/shaders/vanilla");	
	world = std::shared_ptr<World>(new World(RES_POINT + "/data/worlds/random.world"));
	
	//test
	EntityObject fallingCube("../../../res/runtime/models/cube.obj", "../../../res/runtime/textures/metal.jpg", 5.0f, Vector3F(0, 0, 0), Vector3F(0, 0, 0), Vector3F(9, 9, 9));
	world->addEntityObject(fallingCube);
	world->addEntity(player);
	
	KeybindController kc(player, world, wnd);
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
		player.updateMotion(fps);
		fallingCube.updateMotion(fps);
		fpscounter.update();
		deltaTotal += fpscounter.getRange();
		deltas.push_back(fpscounter.getRange());
		
		tk.update();
		wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
		shader.bind();
		
		kc.handleKeybinds();
		fpscounter.reload();
		
		for(unsigned int i = 0; i < world->getMembers().size(); i++)
		{
			Object obj = world->getMembers().at(i);
			obj.render(Mesh::getFromLink(obj.getMeshLink(), allMeshes), Texture::getFromLink(obj.getTextureLink(), allTextures), cam, shader, wnd.getWidth(), wnd.getHeight());
		}
		
		for(unsigned int i = 0; i < world->getEntityObjects().size(); i++)
		{
			EntityObject eo = world->getEntityObjects().at(i);
			eo.render(Mesh::getFromLink(eo.getMeshLink(), allMeshes), Texture::getFromLink(eo.getTextureLink(), allTextures), cam, shader, wnd.getWidth(), wnd.getHeight());
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