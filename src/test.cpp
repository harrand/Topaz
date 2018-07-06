#include "core/listener.hpp"
#include "physics/physics.hpp"
#include "graphics/asset.hpp"
#include "graphics/gui/button.hpp"
#include "graphics/gui/textfield.hpp"
#include "core/scene.hpp"
#include "graphics/skybox.hpp"
#include "core/topaz.hpp"
#include "utility/time.hpp"

void init();

int main()
{
	tz::initialise();
    init();
	tz::terminate();
	return 0;
}


void init()
{
	Window wnd("Topaz Development Window", 0, 30, 800, 600);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // During init, enable debug output
    Font font("../../../res/runtime/fonts/CaviarDreams.ttf", 36);
    Label& label = wnd.emplace_child<Label>(Vector2I{0, 0}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, Vector3F{0.3f, 0.3f, 0.3f}, 0.5f);

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

    Button& test_button = wnd.emplace_child<Button>(Vector2I{0, 200}, Vector2I{100, 50}, font, Vector3F{}, "press me", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    TextField& test_field = wnd.emplace_child<TextField>(Vector2I{0, 300}, Vector2I{100, 50}, font, Vector3F{0.2f, 0.2f, 0.2f}, "Text...", Vector3F{0.1f, 0.1f, 0.1f});

    constexpr float speed = 0.5f;
	Shader render_shader("../../../src/shaders/3D_FullAssetsInstanced");
	Shader gui_shader("../../../src/shaders/gui");
    Camera camera;
    camera.position = {0, 0, -50};
    Scene scene;

    AssetBuffer assets({{"cube_lq", std::make_shared<Mesh>("../../../res/runtime/models/cube.obj")}, {"cube", std::make_shared<Mesh>("../../../res/runtime/models/cube_hd.obj")}}, {{"bricks", std::make_shared<Texture>("../../../res/runtime/textures/bricks.jpg")}}, {{"bricks_normal", std::make_shared<NormalMap>("../../../res/runtime/normalmaps/bricks_normalmap.jpg")}}, {{"bricks_parallax", std::make_shared<ParallaxMap>("../../../res/runtime/parallaxmaps/bricks_parallax.jpg", 0.04f, 0.2f)}}, {{"bricks_displacement", std::make_shared<DisplacementMap>("../../../res/runtime/displacementmaps/bricks_displacement.png")}});
    Asset asset0(assets.find<Mesh>("cube"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), assets.find_parallax_map("bricks_parallax"), assets.find_displacement_map("bricks_displacement"));
    Asset asset1(assets.find_mesh("cube_lq"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), assets.find_parallax_map("bricks_parallax"));
    Asset asset2(assets.find_mesh("cube_lq"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"));
    Asset asset3(assets.find_mesh("cube_lq"), assets.find_texture("bricks"));

    CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "cwd", ".jpg");
    Shader skybox_shader("../../../src/shaders/skybox");
    Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);

    Random rand;
    test_button.set_callback([&scene, &camera, &asset1]()
                             {
                                 scene.emplace_object(Transform{camera.position, {}, {10, 10, 10}}, asset1);
                             });

    std::vector<StaticObject> objects;
    for(float i = 0; i < 50000; i += 1.0f)
    {
        objects.emplace_back(Transform{Vector3F{rand.next_float(-1.0f, 1.0f), rand.next_float(-1.0f, 1.0f), rand.next_float(-1.0f, 1.0f)} * 5000.0f, {}, {5, 5, 5}}, asset2);
    }
   scene.emplace<InstancedStaticObject>(objects);


    long long int time = tz::utility::time::now();
    Timer second_timer;
    TimeProfiler profiler;
	while(!wnd.is_close_requested())
    {
        static float x = 0;
        progress.set_progress(std::abs(std::sin(x += 0.001)));
        profiler.begin_frame();
        second_timer.update();
        if(second_timer.millis_passed(1000.0f))
        {
            using namespace tz::utility::generic::cast;
            label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
            second_timer.reload();
            profiler.reset();
        }
        long long int delta_time = tz::utility::time::now() - time;
        time = tz::utility::time::now();

        wnd.set_render_target();
        wnd.clear();

        profiler.end_frame();

        scene.render(render_shader, camera, {wnd.get_width(), wnd.get_height()});
        scene.update(delta_time / 1000.0f);

        skybox.render(camera, skybox_shader, wnd.get_width(), wnd.get_height());

        wnd.update(gui_shader);
        if(mouse_listener.is_left_clicked() /*&& gui_panel.is_hidden()*/)
        {
            Vector2F delta = mouse_listener.get_mouse_delta_position();
            camera.rotation.y += 0.03 * delta.x;
            camera.rotation.x += 0.03 * delta.y;
            mouse_listener.reload_mouse_delta();
        }
        if(key_listener.is_key_pressed("W"))
            camera.position += camera.forward() * delta_time * speed;
        if(key_listener.is_key_pressed("S"))
            camera.position += camera.backward() * delta_time * speed;
        if(key_listener.is_key_pressed("A"))
            camera.position += camera.left() * delta_time * speed;
        if(key_listener.is_key_pressed("D"))
            camera.position += camera.right() * delta_time * speed;
    }
}