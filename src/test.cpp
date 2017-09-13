#include "engine.hpp"
#include "listeners.hpp"

#ifdef main
#undef main
#endif
int main()
{	
	tz::initialise();
	Window wnd(800, 600, "Topaz Development Window");
	Engine engine(wnd, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = tz::util::cast::fromString<unsigned int>(engine.getResources().getTag("played"));
	constexpr std::size_t shader_id = 0;
	
	KeybindController kc(engine.getCameraR(), engine.getShader(shader_id), engine.getWorldR());
	MouseController mc(engine.getCameraR(), engine.getWorldR());
	engine.getWindowR().registerListener(kc.getKeyListenerR());
	engine.getWindowR().registerListener(mc.getMouseListenerR());
	
	TimeKeeper updater;
	
	//Panel panel(0.0f, 0.0f, 0.5f, 0.5f, Vector3F(1, 0, 0), engine.getDefaultGuiShader());
	//engine.getWindowR().addChild(&panel);
	TTF_Font* example_font = TTF_OpenFont("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextField text(-0.75f, 0.875f, 0.25f, 0.25f, Vector3F(1, 0, 0), example_font, "FPS: 0", engine.getDefaultGuiShader());
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
	tz::terminate();
	return 0;
}