#include "engine.hpp"
#include "listeners.hpp"

#ifdef main
#undef main
#endif
int main()
{	
	LogUtility::message("Initialising camera, player and window...");
	Camera cam;
	Player player(10, cam);
	Window wnd(800, 600, "Topaz Engine - Default World");
	LogUtility::message("Initialising engine...");
	Engine engine(player, wnd, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = CastUtility::fromString<unsigned int>(engine.getResources().getTag("played"));
	std::size_t shader_id = 0;
	
	LogUtility::message("Initialising key controller...");
	KeybindController kc(player, engine.getShader(shader_id), engine.getWorldR(), wnd);
	LogUtility::message("Initialising mouse controller...");
	MouseController mc(player, engine.getWorldR(), wnd);
	
	LogUtility::message("Loading music");
	AudioMusic music("../../../res/runtime/music/music.wav");
	music.play();
	Mix_PauseMusic();
	
	TimeKeeper updater;
	Random rand;
	
	LogUtility::message("Beginning loop...");
	while(!wnd.isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			LogUtility::message("Played: ", seconds++, ", FPS = ", engine.getFPS());
			updater.reload();
		}
		updater.update();
		engine.update(shader_id);
		mc.handleMouse();
		kc.handleKeybinds(engine.getTimeProfiler().getLastDelta(), engine.getProperties().getTag("resources"), engine.getProperties().getTag("controls"));
		mc.getMouseListenerR().reloadMouseDelta();
	}
	return 0;
}