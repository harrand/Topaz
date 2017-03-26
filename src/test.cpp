#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include <iostream>

//Global Heap Variables
std::shared_ptr<World> world;
std::vector<std::shared_ptr<Mesh>> allMeshes;
std::vector<std::shared_ptr<Texture>> allTextures;
std::vector<std::shared_ptr<NormalMap>> allNormalMaps;
std::vector<std::shared_ptr<ParallaxMap>> allParallaxMaps;
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
		
	Shader shader(RES_POINT + "/shaders/noshadows");	
	world = std::shared_ptr<World>(new World(RES_POINT + "/worlds/random.world"));
	world->addEntity(&player);
	
	KeybindController kc(player, world, wnd);
	MouseController mc(player, world, wnd);
	
	DataTranslation dt(RES_POINT + "/resources.data");
	std::cout << "Retrieving assets...\n";
	std::cout << "Retrieved " << dt.retrieveAllData(allMeshes, allTextures, allNormalMaps, allParallaxMaps) << " assets.\n";
	
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
		
		mc.handleMouse();
		kc.handleKeybinds();
		mc.getMouseListener().reloadMouseDelta();
		
		fpscounter.reload();
		
		world->update(fps, cam, shader, wnd.getWidth(), wnd.getHeight(), allMeshes, allTextures, allNormalMaps, allParallaxMaps);
		
		wnd.update();
		wnd.setTitle("Topaz Testing Environment - '" + world->getWorldLink() + "'");
	}
	std::ostringstream strum;
	strum << secondsLifetime;
	timeStorage.editTag("played", strum.str());
	return 0;
}