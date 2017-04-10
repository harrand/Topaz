#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include "audio.hpp"
#include "light.hpp"
#include <iostream>

//Global Heap Variables
std::unique_ptr<World> world;
std::vector<std::unique_ptr<Mesh>> allMeshes;
std::vector<std::unique_ptr<Texture>> allTextures;
std::vector<std::unique_ptr<NormalMap>> allNormalMaps;
std::vector<std::unique_ptr<ParallaxMap>> allParallaxMaps;
//Global Stack Variables
Camera cam;
Player player(10.0f, cam);

// SDL2 defines main.
#ifdef main
#undef main
#endif
int main()
{	
	MDLF timeStorage(RawFile(RES_POINT + "/resources.data"));
	int secondsLifetime = CastUtility::fromString<int>(timeStorage.getTag("played"));
	Window wnd(800, 600, "Topaz Test Environment - Undefined World");
	Shader shader(RES_POINT + "/shaders/noshadows");	
	world = std::unique_ptr<World>(new World(RES_POINT + "/worlds/random.world"));
	world->addEntity(&player);
	KeybindController kc(player, shader, world, wnd);
	MouseController mc(player, world, wnd);
	std::cout << "Retrieving assets...\n";
	std::cout << "Retrieved "<< DataTranslation(RES_POINT + "/resources.data").retrieveAllData(allMeshes, allTextures, allNormalMaps, allParallaxMaps) << " assets.\n";
	TimeKeeper tk;
	TimeProfiler tp;
	unsigned int fps  = 1000;
	
	AudioMusic music(RES_POINT + "/music/music.wav");
	music.play();
	
	std::vector<float> processingThisFrame;

	while(!wnd.isCloseRequested())
	{
		if(tk.millisPassed(1000))
		{
			fps = tp.getFPS();
			std::cout << "\n====== " << secondsLifetime << " seconds ======\n";
			std::cout << "avgdt = " << tp.getDeltaAverage() << " ms, avgFPS = " << fps << " frames per second.\n";
			tp.reset();
			secondsLifetime++;
			tk.reload();
			std::cout << "Camera Position = [" << cam.getPosR().getX() << ", " << cam.getPosR().getY() << ", " << cam.getPosR().getZ() << "].\n";
			float total = 0;
			for(unsigned int i = 0; i < processingThisFrame.size(); i++)
			{
				total += processingThisFrame.at(i);
			}
			std::cout << "Average Rendering Time: " << total/processingThisFrame.size() << " ms. Average Time spent rendering this second: " << total/processingThisFrame.size() * fps << " ms\n";
			processingThisFrame.clear();
		}
		wnd.setRenderTarget();
		tp.beginFrame();
		tk.update();
		wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
		mc.handleMouse();
		kc.handleKeybinds(fps);
		mc.getMouseListener().reloadMouseDelta();
		tp.endFrame();
		TimeKeeper renderTime;
		world->update(fps, cam, shader, wnd.getWidth(), wnd.getHeight(), allMeshes, allTextures, allNormalMaps, allParallaxMaps);
		renderTime.update();
		processingThisFrame.push_back(renderTime.getRange());
		wnd.update();
		wnd.setTitle("Topaz Testing Environment - '" + world->getWorldLink() + "'");
	}
	std::ostringstream strum;
	strum << secondsLifetime;
	timeStorage.editTag("played", strum.str());
	//Mix_FreeMusic(music);
	return 0;
}