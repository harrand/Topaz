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
	KeybindController kc(player, engine.getShader(0), engine.getWorldR(), wnd);
	MouseController mc(player, engine.getWorldR(), wnd);
	
	AudioMusic music(RES_POINT + "/music/music.wav");
	music.play();
	Mix_PauseMusic();
	
	while(!wnd.isCloseRequested())
	{
		engine.update(cam, 0, mc, kc);
	}
	return 0;
}