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

class FPSToggleCommand : public Command
{
public:
	FPSToggleCommand(TextLabel& fps_label): fps_label(fps_label){}
	virtual void operator()([[maybe_unused]] const std::vector<std::string>& args){fps_label.setHidden(!fps_label.isHidden());}
	TextLabel& fps_label;
};

class ExitGuiCommand : public Command
{
public:
	ExitGuiCommand(Panel& gui_panel): gui_panel(gui_panel){}
	virtual void operator()([[maybe_unused]] const std::vector<std::string>& args){gui_panel.setHidden(true);}
	Panel& gui_panel;
};

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
	TextLabel text(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "FPS: ...", engine.getDefaultGuiShader());
	FPSToggleCommand toggle(text);
	Panel gui_panel(0.0f, 0.0f, wnd.getWidth(), wnd.getHeight(), Vector4F(0.4f, 0.4f, 0.4f, 0.5f), engine.getDefaultGuiShader());
	gui_panel.setHidden(true);
	ExitGuiCommand exit(gui_panel);
	TextLabel gui_title(0.0f, wnd.getHeight() - 50, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Main Menu", engine.getDefaultGuiShader());
	Button test_button(0.0f, 2 * text.getHeight(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.7, 0.7, 1.0), Vector3F(0, 0, 0), example_font, "Hide/Show", engine.getDefaultGuiShader(), mc.getMouseListenerR());
	Button exit_gui_button(wnd.getWidth() - 50, wnd.getHeight() - 50, Vector4F(1, 1, 1, 1), Vector4F(1.0, 0, 0, 1.0), Vector3F(0, 0, 0), example_font, "X", engine.getDefaultGuiShader(), mc.getMouseListenerR());
	engine.getWindowR().addChild(&text);
	engine.getWindowR().addChild(&gui_panel);
	gui_panel.addChild(&gui_title);
	gui_panel.addChild(&test_button);
	gui_panel.addChild(&exit_gui_button);
	test_button.getOnMouseClickR() = &toggle;
	exit_gui_button.getOnMouseClickR() = &exit;	
	
	while(!engine.getWindowR().isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			text.setText("FPS: " + tz::util::cast::toString(engine.getFPS()));
			updater.reload();
			seconds++;
		}
		if(kc.getKeyListenerR().catchKeyPressed("Escape"))
			gui_panel.setHidden(!gui_panel.isHidden());
		updater.update();
		engine.update(shader_id);
		mc.handleMouse();
		kc.handleKeybinds(engine.getTimeProfiler().getLastDelta(), engine.getProperties().getTag("resources"), engine.getProperties().getTag("controls"));
		mc.getMouseListenerR().reloadMouseDelta();
	}
}