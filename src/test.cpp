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
	Engine engine(player, wnd);
	LogUtility::message("Initialising key and mouse controllers...");
	size_t shader_id = 0;
	KeybindController kc(player, engine.getShader(shader_id), engine.getWorldR(), wnd);
	MouseController mc(player, engine.getWorldR(), wnd);
	
	AudioMusic music(RES_POINT + "/music/music.wav");
	music.play();
	Mix_PauseMusic();
	
	while(!wnd.isCloseRequested())
	{
		engine.update(shader_id, mc, kc);
	}
	return 0;
}