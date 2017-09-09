#include "engine.hpp"
#include "listeners.hpp"

#ifdef main
#undef main
#endif
int main()
{	
	tz::initialise();
	tz::util::log::message("Initialising camera, player and window...");
	Camera cam;
	Window sdl_window_pointer(800, 600, "Topaz Development Test Program");
	tz::util::log::message("Initialising engine...");
	Engine engine(cam, sdl_window_pointer, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = tz::util::cast::fromString<unsigned int>(engine.getResources().getTag("played"));
	constexpr std::size_t shader_id = 0;
	
	tz::util::log::message("Initialising key controller...");
	KeybindController kc(cam, engine.getShader(shader_id), engine.getWorldR());
	tz::util::log::message("Initialising mouse controller...");
	MouseController mc(cam, engine.getWorldR());
	sdl_window_pointer.registerListener(kc.getKeyListenerR());
	sdl_window_pointer.registerListener(mc.getMouseListenerR());
	
	tz::util::log::message("Loading music");
	AudioMusic music("../../../res/runtime/music/music.wav");
	music.play();
	Mix_PauseMusic();
	
	TimeKeeper updater;
	
	tz::util::log::message("Beginning loop...");
	while(!sdl_window_pointer.isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			tz::util::log::message("Played: ", seconds++, ", FPS = ", engine.getFPS());
			updater.reload();
		}
		updater.update();
		engine.update(shader_id);
		mc.handleMouse();
		kc.handleKeybinds(engine.getTimeProfiler().getLastDelta(), engine.getProperties().getTag("resources"), engine.getProperties().getTag("controls"));
		mc.getMouseListenerR().reloadMouseDelta();
	}
	tz::terminate();
	return 0;
}