#include "listeners.hpp"
#include "timekeeper.hpp"
#include "command.hpp"
#include "audio.hpp"
#include "light.hpp"
#include <iostream>

//Global Heap Variables
std::vector<std::unique_ptr<Mesh>> allMeshes;
std::vector<std::unique_ptr<Texture>> allTextures;
std::vector<std::unique_ptr<NormalMap>> allNormalMaps;
std::vector<std::unique_ptr<ParallaxMap>> allParallaxMaps;
//Global Stack Variables
World world(RES_POINT + "/worlds/random.world");
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
	Shader skyboxShader(RES_POINT + "/shaders/skybox");
	world.addEntity(player);
	KeybindController kc(player, shader, world, wnd);
	MouseController mc(player, world, wnd);
	LogUtility::message("Loading assets...");
	LogUtility::message("Retrieved " + CastUtility::toString<unsigned int>(DataTranslation(RES_POINT + "/resources.data").retrieveAllData(allMeshes, allTextures, allNormalMaps, allParallaxMaps)) + " assets.");
	std::string texturesDirectory = RES_POINT + "/textures/";
	//CubeMap blood(texturesDirectory + "blood_rt.png", texturesDirectory + "blood_lf.png", texturesDirectory + "blood_up.png", texturesDirectory + "blood_dn.png", texturesDirectory + "blood_bk.png", texturesDirectory + "blood_ft.png");
	CubeMap greenhaze(texturesDirectory + "greenhaze_rt.png", texturesDirectory + "greenhaze_lf.png", texturesDirectory + "greenhaze_up.png", texturesDirectory + "greenhaze_dn.png", texturesDirectory + "greenhaze_bk.png", texturesDirectory + "greenhaze_ft.png");
	Skybox box("../../../res/runtime/models/skybox.obj", greenhaze);
	
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
			LogUtility::silent("\n====== " + CastUtility::toString<int>(secondsLifetime) + " seconds ======\navgdt = " + CastUtility::toString<float>(tp.getDeltaAverage()) + " ms, avgFPS = " + CastUtility::toString<int>(fps) + " fps.");
			tp.reset();
			secondsLifetime++;
			tk.reload();
			LogUtility::silent("Camera Position = [" + CastUtility::toString<float>(cam.getPosR().getX()) + ", " + CastUtility::toString<float>(cam.getPosR().getY()) + ", " + CastUtility::toString<float>(cam.getPosR().getZ()) + "].");
		}
		wnd.setRenderTarget();
		tp.beginFrame();
		tk.update();
		wnd.clear(0.0f, 0.0f, 0.0f, 1.0f);
		mc.handleMouse();
		kc.handleKeybinds(tp.getLastDelta());
		mc.getMouseListenerR().reloadMouseDelta();
		tp.endFrame();
		TimeKeeper renderTime;
		
		box.render(cam, skyboxShader, allMeshes, wnd.getWidth(), wnd.getHeight());
		world.update(fps, cam, shader, wnd.getWidth(), wnd.getHeight(), allMeshes, allTextures, allNormalMaps, allParallaxMaps);
		renderTime.update();
		processingThisFrame.push_back(renderTime.getRange());
		wnd.update();
		wnd.setTitle("Topaz Testing Environment - '" + world.getWorldLink() + "'");
	}
	std::ostringstream strum;
	strum << secondsLifetime;
	timeStorage.editTag("played", strum.str());
	return 0;
}