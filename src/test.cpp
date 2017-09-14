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
	TextLabel text(0.0f, 0.0f, Vector3F(1, 1, 1), {}, example_font, "FPS: ...", engine.getDefaultGuiShader());
	Button test_button(400.0f, 400.0f, Vector3F(0, 1, 1), {}, example_font, "Press me", engine.getDefaultGuiShader(), mc.getMouseListenerR());
	engine.getWindowR().addChild(&text);
	engine.getWindowR().addChild(&test_button);
	
	
	while(!engine.getWindowR().isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			text.setText("FPS: " + tz::util::cast::toString(engine.getFPS()));
			updater.reload();
			seconds++;
		}
		if(kc.getKeyListenerR().catchKeyPressed("Escape"))
			text.setHidden(!text.isHidden());
		updater.update();
		engine.update(shader_id);
		mc.handleMouse();
		kc.handleKeybinds(engine.getTimeProfiler().getLastDelta(), engine.getProperties().getTag("resources"), engine.getProperties().getTag("controls"));
		mc.getMouseListenerR().reloadMouseDelta();
	}
}