#include "engine.hpp"
#include "listener.hpp"

void init();
#ifdef main
#undef main
#endif
int main()
{
	tz::initialise();
	init();
	tz::terminate();
	return 0;
}

void init()
{
	Window wnd(800, 600, "Topaz Development Window");
	Engine engine(wnd, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = tz::util::cast::fromString<unsigned int>(engine.getResources().getTag("played"));
	constexpr std::size_t shader_id = 0;
	
	KeybindController kc(engine.getCameraR(), engine.getShader(shader_id), engine.getWorldR());
	MouseController mc(engine.getCameraR(), engine.getWorldR());
	engine.getWindowR().registerListener(kc.getKeyListenerR());
	engine.getWindowR().registerListener(mc.getMouseListenerR());
	
	TimeKeeper updater;
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextField text(0.0f, 0.0f, Vector3F(1, 0, 0), {}, example_font, "FPS: 0", engine.getDefaultGuiShader());
	engine.getWindowR().addChild(&text);
	
	while(!engine.getWindowR().isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			text.setText("FPS: " + tz::util::cast::toString(engine.getFPS()));
			seconds++;
			updater.reload();
		}
		if(kc.getKeyListenerR().catchKeyPressed("Q"))
			tz::util::log::message("Played: ", seconds, ", FPS = ", engine.getFPS());
		if(kc.getKeyListenerR().catchKeyPressed("Escape"))
			text.setHidden(!text.isHidden());
		updater.update();
		engine.update(shader_id);
		mc.handleMouse();
		kc.handleKeybinds(engine.getTimeProfiler().getLastDelta(), engine.getProperties().getTag("resources"), engine.getProperties().getTag("controls"));
		mc.getMouseListenerR().reloadMouseDelta();
	}
}