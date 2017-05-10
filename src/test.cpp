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
	LogUtility::message("Retrieved ", DataTranslation(RES_POINT + "/resources.data").retrieveAllData(allMeshes, allTextures, allNormalMaps, allParallaxMaps), " assets.");
	CubeMap greenhaze(RES_POINT + "/textures/", "greenhaze", ".png");
	Skybox box("../../../res/runtime/models/skybox.obj", greenhaze);
	
	TimeKeeper tk;
	TimeProfiler tp;
	unsigned int fps  = 1000;
	
	AudioMusic music(RES_POINT + "/music/music.wav");
	music.play();

	while(!wnd.isCloseRequested())
	{
		if(tk.millisPassed(1000))
		{
			fps = tp.getFPS();
			LogUtility::silent("\n====== ", secondsLifetime, " seconds ======\navgdt = ", tp.getDeltaAverage(), " ms, avgFPS = ", fps, " fps.");
			tp.reset();
			secondsLifetime++;
			tk.reload();
			LogUtility::silent("Camera Position = ", StringUtility::format(StringUtility::devectoriseList3<float>(cam.getPos())));
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
		wnd.update();
		wnd.setTitle("Topaz Testing Environment - '" + world.getWorldLink() + "'");
		
		GLenum error;
		if((error = glGetError()) != GL_NO_ERROR)
			LogUtility::error("OpenGL Error: ", error, "\n");
	}
	std::ostringstream strum;
	strum << secondsLifetime;
	timeStorage.editTag("played", strum.str());
	return 0;
}