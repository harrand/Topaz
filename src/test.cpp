#include "listener.hpp"
#include "audio/audio.hpp"
#include "physics/physics.hpp"
#include "data.hpp"
#include "gui_display.hpp"
#include "scene.hpp"

namespace tz
{
    void initialise()
    {
        tz::util::log::message("Initialising Topaz...");
        SDL_Init(SDL_INIT_EVERYTHING);
        tz::util::log::message("Initialised SDL2.");
        tz::audio::initialise();
        tz::util::log::message("Initialised Topaz. Ready to receive OpenGL context...");
    }

    void terminate()
    {
        tz::util::log::message("Terminating Topaz...");
        tz::graphics::terminate();
        tz::audio::terminate();
        SDL_Quit();
        tz::util::log::message("Terminated SDL2.");
        tz::util::log::message("Terminated Topaz.");
    }
}

void init();
void test();
#ifdef main
#undef main
#endif

int main()
{
	tz::initialise();
	//test();
	init();
	tz::terminate();
	return 0;
}

void init()
{
	Window wnd("Topaz Development Window", 0, 0, 800, 600);
    Font font("../../../res/runtime/fonts/CaviarDreams.ttf", 32);
    Label& label = *wnd.emplace_child<Label>(Vector2<int>{0, 0}, font, Vector3F{0.0f, 0.3f, 0.0f}, "hello, world.");
    label.set_local_position_normalised_space({0.5f, 0.5f});
    wnd.centre_position();

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

	Shader render_shader("../../../src/shaders/noshadows");
	Shader gui_shader("../../../src/shaders/gui");
    Camera camera;

    Scene scene;
    AssetBuffer assets({std::make_shared<Mesh>("../../../res/runtime/models/cube_hd.obj")}, {std::make_shared<Texture>("../../../res/runtime/textures/bricks.jpg")});
    Asset asset(assets.meshes.front(), assets.textures.front());
    SceneObject& test_object = scene.emplace_object(Transform{{0, 0, 0}, {}, {10, 10, 10}}, asset);
	while(!wnd.is_close_requested())
    {
        wnd.set_render_target();
        wnd.clear();
        scene.render(render_shader, camera, {wnd.get_width(), wnd.get_height()});
        wnd.update(gui_shader);
        if(mouse_listener.is_left_clicked() /*&& gui_panel.is_hidden()*/)
        {
            Vector2F delta = mouse_listener.get_mouse_delta_pos();
            camera.rotation.y += 0.03 * delta.x;
            camera.rotation.x -= 0.03 * delta.y;
            mouse_listener.reload_mouse_delta();
        }
        static int x = 0;
        test_object.transform.position.z = 100 * std::sin(++x / 20000.0f);
        test_object.transform.rotation.y = x / 1000.0f;
    }
}

void test()
{
	using namespace std::chrono_literals;
	using namespace tz::util::log;
	auto cls = [](){system("cls");};
	
	message("Playing test.wav asynchronously...");
	AudioClip test_wav("../../../res/runtime/music/test.wav");
	test_wav.play();
	message("Waiting 5 seconds...");
	std::this_thread::sleep_for(5s);
	AudioClip test_wav_copy(test_wav);
	cls();
	message("Playing copy of test wav...");
	test_wav_copy.play();
	message("Waiting 5 seconds...");
	std::this_thread::sleep_for(5s);
	cls();
	message("Re-playing origin copy of test.wav...");
	test_wav.play();
	message("Waiting 5 seconds");
	cls();
	AudioClip test_wav_moved = std::move(test_wav);
	message("Moved test.wav to a new instance, playing once more...");
	test_wav_moved.play();
	message("Waiting 5 seconds...");
	std::this_thread::sleep_for(5s);
	cls();
	// test RNG
	Random rand;
	MersenneTwister mt;
	for(unsigned int i = 0; i < 100; i++)
	{
		message("Default Random: ", rand.operator()<int>(0, 100), ", MersenneTwister: ", mt.operator()<int>(0, 100));
	}
	cls();
	message("TESTING COMPLETE");
}