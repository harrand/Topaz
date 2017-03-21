#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include <iostream>

//Global Heap Variables
std::shared_ptr<World> world;
std::vector<std::shared_ptr<Mesh>> allMeshes;
std::vector<std::shared_ptr<Texture>> allTextures;
std::vector<std::shared_ptr<NormalMap>> allNormalMaps;
//Global Stack Variables
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
	
	Window wnd(800, 600, "Topaz Test Environment - Undefined World");
	TimeKeeper tk, fpscounter;
	
	std::cout << "'Topaz Testing Environment created.\n";
	
	Shader shader(RES_POINT + "/shaders/vanilla");	
	world = std::shared_ptr<World>(new World(RES_POINT + "/data/worlds/random.world"));
	world->addEntity(&player);
	
	KeybindController kc(player, world, wnd);
	MouseController mc(player, world, wnd);
	DataTranslation dt(RES_POINT + "/resources.data");
	
	std::map<std::string, std::string> models = dt.retrieveModels(), textures = dt.retrieveTextures(), normalmaps = dt.retrieveNormalMaps();
	
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
	std::cout << "Retrieving normalmaps...\n";
	for(it_type iterator = normalmaps.begin(); iterator != normalmaps.end(); iterator++)
	{
		std::cout << "Initialising a normalmap with the link " << iterator->first << ".\n";
		allNormalMaps.push_back(std::shared_ptr<NormalMap>(new NormalMap(iterator->first)));
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
		wnd.setRenderTarget();
		fpscounter.update();
		deltaTotal += fpscounter.getRange();
		deltas.push_back(fpscounter.getRange());
		
		tk.update();
		wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
		shader.bind();
		
		kc.handleKeybinds();
		mc.handleMouse();
		
		fpscounter.reload();
		
		world->update(fps, cam, shader, wnd.getWidth(), wnd.getHeight(), allMeshes, allTextures, allNormalMaps);
		for(unsigned int i = 0; i < world->getEntityObjects().size(); i++)
		{
			std::shared_ptr<EntityObject> eo = world->getEntityObjects().at(i);
			eo->applyForce("playerattraction", Force(player.getPosition() - eo->getPosition()) * 100);
		}
		
		wnd.update();
		wnd.setTitle("Topaz Testing Environment - '" + world->getWorldLink() + "'");
	}
	std::ostringstream strum;
	strum << secondsLifetime;
	timeStorage.deleteTag("played");
	timeStorage.addTag("played", strum.str());
	return 0;
}