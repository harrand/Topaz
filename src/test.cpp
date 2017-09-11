#include "engine.hpp"
#include "listeners.hpp"

#ifdef main
#undef main
#endif
int main()
{	
	tz::initialise();
	tz::util::log::message("Initialising camera, player and window...");
	tz::util::log::message("Initialising engine...");
	Window wnd(800, 600, "Topaz Development Window");
	Engine engine(wnd, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = tz::util::cast::fromString<unsigned int>(engine.getResources().getTag("played"));
	constexpr std::size_t shader_id = 0;
	
	tz::util::log::message("Initialising key controller...");
	KeybindController kc(engine.getCameraR(), engine.getShader(shader_id), engine.getWorldR());
	tz::util::log::message("Initialising mouse controller...");
	MouseController mc(engine.getCameraR(), engine.getWorldR());
	engine.getWindowR().registerListener(kc.getKeyListenerR());
	engine.getWindowR().registerListener(mc.getMouseListenerR());
	
	tz::util::log::message("Loading music");
	AudioMusic music("../../../res/runtime/music/music.wav");
	music.play();
	Mix_PauseMusic();
	
	TimeKeeper updater;
	
	Panel panel(0.0f, 0.0f, 0.5f, 0.5f);
	engine.getWindowR().addChild(&panel);
	
	tz::util::log::message("Beginning loop...");
	while(!engine.getWindowR().isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			tz::util::log::message("Played: ", seconds++, ", FPS = ", engine.getFPS());
			updater.reload();
		}
		panel.setHidden(!kc.getKeyListener().isKeyPressed("G"));
		updater.update();
		engine.update(shader_id);
		mc.handleMouse();
		kc.handleKeybinds(engine.getTimeProfiler().getLastDelta(), engine.getProperties().getTag("resources"), engine.getProperties().getTag("controls"));
		mc.getMouseListenerR().reloadMouseDelta();
	}
	tz::terminate();
	return 0;
}