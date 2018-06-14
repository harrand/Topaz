#include "listener.hpp"
#include "physics/physics.hpp"
#include "asset.hpp"
#include "gui_display.hpp"
#include "scene.hpp"
#include "graphics/skybox.hpp"
#include "topaz.hpp"
#include "time.hpp"

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
    Font font("../../../res/runtime/fonts/CaviarDreams.ttf", 36);
    Label& label = wnd.emplace_child<Label>(Vector2<int>{0, 0}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    //label.set_local_position_normalised_space({0.5f, 0.5f});

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

    constexpr float speed = 0.5f;
	Shader render_shader("../../../src/shaders/3D_FullAssets");
	Shader gui_shader("../../../src/shaders/gui");
    Camera camera;
    camera.position = {0, 0, -50};
    Scene scene;

    AssetBuffer assets({std::make_shared<Mesh>("../../../res/runtime/models/cube_hd.obj")}, {std::make_shared<Texture>("../../../res/runtime/textures/bricks.jpg")}, {std::make_shared<NormalMap>("../../../res/runtime/normalmaps/bricks_normalmap.jpg")}, {std::make_shared<ParallaxMap>("../../../res/runtime/parallaxmaps/bricks_parallax.jpg", 0.04f, 0.2f)}, {std::make_shared<DisplacementMap>("../../../res/runtime/displacementmaps/bricks_displacement.png")});
    Asset asset0(assets.meshes.front(), assets.textures.front(), assets.normal_maps.front(), assets.parallax_maps.front(), assets.displacement_maps.front());
    Asset asset1(assets.meshes.front(), assets.textures.front(), assets.normal_maps.front(), assets.parallax_maps.front());
    Asset asset2(assets.meshes.front(), assets.textures.front(), assets.normal_maps.front());
    Asset asset3(assets.meshes.front(), assets.textures.front());
    SceneObject& test_object0 = scene.emplace_object(Transform{Vector3F{-50, 0, 0}, Vector3F{0, 0, 0}, Vector3F{10, 10, 10}}, asset0);
    SceneObject& test_object1 = scene.emplace_object(Transform{Vector3F{-25, 0, 0}, Vector3F{0, 0, 0}, Vector3F{5, 5, 5}}, asset1);
    SceneObject& test_object2 = scene.emplace_object(Transform{Vector3F{0, 0, 0}, Vector3F{0, 0, 0}, Vector3F{7, 7, 7}}, asset2);
    SceneObject& test_object3 = scene.emplace_object(Transform{{25}, {}, {10, 10, 10}}, asset3);

    CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "cwd", ".jpg");
    Shader skybox_shader("../../../src/shaders/skybox");
    Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);

    long long int time = tz::time::now();
    Timer second_timer;
    TimeProfiler profiler;
	while(!wnd.is_close_requested())
    {
        profiler.begin_frame();
        second_timer.update();
        if(second_timer.millis_passed(1000.0f))
        {
            using namespace tz::util::cast;
            label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
            second_timer.reload();
            profiler.reset();
        }
        long long int delta_time = tz::time::now() - time;
        time = tz::time::now();

        wnd.set_render_target();
        wnd.clear();

        profiler.end_frame();

        scene.render(render_shader, camera, {wnd.get_width(), wnd.get_height()});

        skybox.render(camera, skybox_shader, *assets.meshes.front(), wnd.get_width(), wnd.get_height());

        wnd.update(gui_shader);
        if(mouse_listener.is_left_clicked() /*&& gui_panel.is_hidden()*/)
        {
            Vector2F delta = mouse_listener.get_mouse_delta_pos();
            camera.rotation.y += 0.03 * delta.x;
            camera.rotation.x -= 0.03 * delta.y;
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
        static int x = 0;
        x++;
        float rot = tz::consts::pi * std::sin(x / 1000.0f);
        test_object0.transform.rotation.y = rot;
        test_object1.transform.rotation.y = rot;
        test_object2.transform.rotation.y = rot;
        test_object3.transform.rotation.y = rot;
    }
}