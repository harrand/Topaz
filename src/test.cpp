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
	virtual void operator()([[maybe_unused]] const std::vector<std::string>& args){gui_panel.setHidden(!gui_panel.isHidden());}
	Panel& gui_panel;
};

class RenderSkyboxCommand : public Command
{
public:
	RenderSkyboxCommand(Skybox& skybox, Camera& camera, Shader& shader, const std::vector<std::unique_ptr<Mesh>>& all_meshes, Window& wnd): skybox(skybox), camera(camera), shader(shader), all_meshes(all_meshes), wnd(wnd){}
	virtual void operator()([[maybe_unused]] const std::vector<std::string>& args)
	{
		skybox.render(camera, shader, all_meshes, wnd.getWidth(), wnd.getHeight());
	}
	Skybox& skybox;
	Camera& camera;
	Shader& shader;
	const std::vector<std::unique_ptr<Mesh>>& all_meshes;
	Window& wnd;
};

void init()
{
	Window wnd(800, 600, "Topaz Development Window");
	Engine engine(wnd, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = tz::util::cast::fromString<unsigned int>(engine.getResources().getTag("played"));
	float rotational_speed = tz::util::cast::fromString<float>(engine.getResources().getTag("rotational_speed"));
	constexpr std::size_t shader_id = 0;
	
	KeyListener key_listener;
	MouseListener mouse_listener;
	engine.getWindowR().registerListener(key_listener);
	engine.getWindowR().registerListener(mouse_listener);
	CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "greenhaze", ".png");
	Shader skybox_shader("../../../src/shaders/skybox");
	
	TimeKeeper updater;
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextLabel text(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "FPS: ...", engine.getDefaultGuiShader());
	FPSToggleCommand toggle(text);
	Panel gui_panel(-1.0f, -1.0f, 1.0f, 1.0f, Vector4F(0.4f, 0.4f, 0.4f, 0.5f), engine.getDefaultGuiShader());
	gui_panel.setUsingProportionalPositioning(true);
	gui_panel.setHidden(true);
	ExitGuiCommand exit(gui_panel);
	TextLabel gui_title(0.0f, wnd.getHeight() - 50, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Main Menu", engine.getDefaultGuiShader());
	Button test_button(0.0f, 2 * text.getHeight(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.7, 0.7, 1.0), Vector3F(0, 0, 0), example_font, "Hide/Show", engine.getDefaultGuiShader(), mouse_listener);
	Button creation_toggle(0.0f, 2 * text.getHeight() + 2 * test_button.getHeight(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.7, 0.7, 1.0), Vector3F(0, 0, 0), example_font, "Create Objects", engine.getDefaultGuiShader(), mouse_listener);
	Button exit_gui_button(wnd.getWidth() - 50, wnd.getHeight() - 50, Vector4F(1, 1, 1, 1), Vector4F(1.0, 0, 0, 1.0), Vector3F(0, 0, 0), example_font, "X", engine.getDefaultGuiShader(), mouse_listener);
	Panel creation_panel(-1.0f, -1.0f, 1.0f, 1.0f, Vector4F(0.4f, 0.4f, 0.4f, 1.0f), engine.getDefaultGuiShader());
	ExitGuiCommand exit_creation(creation_panel);
	creation_panel.setUsingProportionalPositioning(true);
	creation_panel.setHidden(true);
	engine.getWindowR().addChild(&creation_panel);
	engine.getWindowR().addChild(&text);
	engine.getWindowR().addChild(&gui_panel);
	gui_panel.addChild(&gui_title);
	gui_panel.addChild(&test_button);
	gui_panel.addChild(&exit_gui_button);
	gui_panel.addChild(&creation_toggle);
	test_button.getOnMouseClickR() = &toggle;
	exit_gui_button.getOnMouseClickR() = &exit;
	creation_toggle.getOnMouseClickR() = &exit_creation;
	
	
	Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);
	RenderSkyboxCommand render_skybox(skybox, engine.getCameraR(), skybox_shader, engine.getMeshes(), wnd);
	engine.getCommandExecutorR().registerCommand(&render_skybox);

	while(!engine.getWindowR().isCloseRequested())
	{
		if(updater.millisPassed(1000))
		{
			text.setText("FPS: " + tz::util::cast::toString(engine.getFPS()));
			updater.reload();
			seconds++;
		}
	
		float multiplier = tz::util::cast::fromString<float>(MDLF(RawFile(engine.getProperties().getTag("resources"))).getTag("speed"));
		if(key_listener.isKeyPressed("W"))
			engine.getCameraR().getPositionR() += (engine.getCameraR().getForward() * multiplier * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("S"))
			engine.getCameraR().getPositionR() += (engine.getCameraR().getBackward() * multiplier * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("A"))
			engine.getCameraR().getPositionR() += (engine.getCameraR().getLeft() * multiplier * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("D"))
			engine.getCameraR().getPositionR() += (engine.getCameraR().getRight() * multiplier * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("Space"))
			engine.getCameraR().getPositionR() += (Vector3F(0, 1, 0) * multiplier * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("Z"))
			engine.getCameraR().getPositionR() += (Vector3F(0, -1, 0) * multiplier * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("I"))
			engine.getCameraR().getRotationR() += (Vector3F(1.0f/360.0f, 0, 0) * multiplier * 5 * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("K"))
			engine.getCameraR().getRotationR() += (Vector3F(-1.0f/360.0f, 0, 0) * multiplier * 5 * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("J"))
			engine.getCameraR().getRotationR() += (Vector3F(0, -1.0f/360.0f, 0) * multiplier * 5 * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("L"))
			engine.getCameraR().getRotationR() += (Vector3F(0, 1.0f/360.0f, 0) * multiplier * 5 * engine.getTimeProfiler().getLastDelta());
		if(key_listener.isKeyPressed("R"))
		{
			engine.getCameraR().getPositionR() = engine.getWorldR().getSpawnPoint();
			engine.getCameraR().getRotationR() = engine.getWorldR().getSpawnOrientation();
		}
		if(key_listener.catchKeyPressed("Escape"))
			gui_panel.setHidden(!gui_panel.isHidden());
		exit_gui_button.getXR() = wnd.getWidth() - (exit_gui_button.getWidth() * 2);
		exit_gui_button.getYR() = wnd.getHeight() - (exit_gui_button.getHeight() * 2);
		gui_title.getYR() = wnd.getHeight() - (gui_title.getHeight() * 2);
		updater.update();
		engine.update(shader_id);
		if(mouse_listener.isLeftClicked() && gui_panel.isHidden())
		{
			Vector3F& orientation = engine.getCameraR().getRotationR();
			Vector2F delta = mouse_listener.getMouseDeltaPos();
			orientation.getYR() += (rotational_speed * delta.getX());
			orientation.getXR() -= (rotational_speed * delta.getY());
			mouse_listener.reloadMouseDelta();
		}
	}
	engine.getResourcesR().editTag("played", tz::util::cast::toString(seconds));
}