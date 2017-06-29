#include "engine.hpp"

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
	std::size_t shader_id = 0;
	LogUtility::message("Initialising key controller...");
	KeybindController kc(player, engine.getShader(shader_id), engine.getWorldR(), wnd);
	LogUtility::message("Initialising mouse controller...");
	MouseController mc(player, engine.getWorldR(), wnd);
	
	LogUtility::message("Loading music");
	AudioMusic music("../../../res/runtime/music/music.wav");
	music.play();
	Mix_PauseMusic();
	
	/*
	Shader skyboxShader(engine.getProperties().getTag("skybox_shader_path"));
	CubeMap cm(engine.getProperties().getTag("skybox_directory"), engine.getProperties().getTag("skybox_name"), ".png");
	Skybox skybox(engine.getResources().getTag("cube.path"), cm);
	engine.getWorldR().getSkyboxShaderR() = skyboxShader;
	engine.getWorldR().getSkyboxR() = skybox;
	*/
	
	LogUtility::message("Beginning loop...");
	while(!wnd.isCloseRequested())
	{
		engine.update(shader_id, mc, kc);
	}
	return 0;
}