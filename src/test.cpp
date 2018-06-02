#include "listener.hpp"
#include "physics/physics.hpp"
#include "asset.hpp"
#include "gui_display.hpp"
#include "scene.hpp"
#include "graphics/skybox.hpp"
#include "topaz.hpp"

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
    Label& label = wnd.emplace_child<Label>(Vector2<int>{0, 0}, font, Vector3F{0.0f, 0.3f, 0.0f}, "hello there");
    label.set_local_position_normalised_space({0.5f, 0.5f});

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

	Shader render_shader("../../../src/shaders/3D_FullAssets");
	Shader gui_shader("../../../src/shaders/gui");
    Camera camera;
    Scene scene;

    AssetBuffer assets({std::make_shared<Mesh>("../../../res/runtime/models/cube_hd.obj")}, {std::make_shared<Texture>("../../../res/runtime/textures/bricks.jpg")}, {std::make_shared<NormalMap>("../../../res/runtime/normalmaps/bricks_normalmap.jpg")}, {std::make_shared<ParallaxMap>("../../../res/runtime/parallaxmaps/bricks_parallax.jpg")}, {std::make_shared<DisplacementMap>("../../../res/runtime/displacementmaps/bricks_displacement.png")});
    Mesh skybox_mesh = assets.emplace<Mesh>(std::string("../../../res/runtime/models/skybox.obj"));
    Asset asset(assets.meshes.front(), assets.textures.front(), assets.normal_maps.front());
    SceneObject& test_object = scene.emplace_object(Transform{{0, 0, 0}, {}, {10, 10, 10}}, asset);

    CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "cwd", ".jpg");
    Shader skybox_shader("../../../src/shaders/skybox");
    Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);

	while(!wnd.is_close_requested())
    {
        wnd.set_render_target();
        wnd.clear();

        scene.render(render_shader, camera, {wnd.get_width(), wnd.get_height()});

        skybox.render(camera, skybox_shader, skybox_mesh, wnd.get_width(), wnd.get_height());

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
        test_object.transform.rotation.y = tz::consts::pi * std::sin(x / 1000.0f);
    }
}