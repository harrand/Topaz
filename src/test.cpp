#include "core/listener.hpp"
#include "physics/physics.hpp"
#include "graphics/asset.hpp"
#include "graphics/gui/button.hpp"
#include "graphics/gui/textfield.hpp"
#include "core/scene.hpp"
#include "graphics/skybox.hpp"
#include "core/topaz.hpp"
#include "utility/time.hpp"
#include "graphics/frame_buffer.hpp"

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
    // During init, enable debug output
    Font font("../../../res/runtime/fonts/CaviarDreams.ttf", 36);
    Label& label = wnd.emplace_child<Label>(Vector2I{0, 0}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, Vector3F{0.3f, 0.3f, 0.3f}, 0.5f);

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

    Button& test_button = wnd.emplace_child<Button>(Vector2I{0, 200}, Vector2I{100, 50}, font, Vector3F{}, "press me", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    Button& wireframe_button = wnd.emplace_child<Button>(Vector2I{0, 100}, Vector2I{100, 50}, font, Vector3F{}, "toggle wireframe", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    wireframe_button.set_callback([](){static bool wireframe = false;wireframe = !wireframe;tz::graphics::enable_wireframe_render(wireframe);});

    constexpr float speed = 0.5f;
	Shader render_shader("../../../src/shaders/3D_FullAssetsInstancedShadowsBloom");

	Shader gui_shader("../../../src/shaders/Gui");
    Shader hdr_gui_shader("../../../src/shaders/Gui_HDR");
    Camera camera;
    camera.position = {0, 0, -50};
    Scene scene;
    //scene.add_point_light({{0, 0, 0}, {0, 1, 0}, 5000000.0f});
    scene.add_directional_light({{0, 1, 0}, {1, 1, 1}, 5.0f});

    AssetBuffer assets;
    assets.emplace<Mesh>("cube_lq", "../../../res/runtime/models/cube.obj");
    assets.emplace<Mesh>("cube", "../../../res/runtime/models/cube_hd.obj");
    assets.emplace<Mesh>("monkey", "../../../res/runtime/models/monkeyhead.obj");
    assets.emplace<Texture>("bricks", "../../../res/runtime/textures/bricks.jpg");
    assets.emplace<NormalMap>("bricks_normal", "../../../res/runtime/normalmaps/bricks_normalmap.jpg");
    assets.emplace<ParallaxMap>("bricks_parallax", "../../../res/runtime/parallaxmaps/bricks_parallax.jpg");
    assets.emplace<DisplacementMap>("bricks_displacement", "../../../res/runtime/displacementmaps/bricks_displacement.png");
    Asset asset0(assets.find<Mesh>("cube"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), assets.find_parallax_map("bricks_parallax"), assets.find_displacement_map("bricks_displacement"));
    Asset asset1(assets.find_mesh("cube_lq"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), assets.find_parallax_map("bricks_parallax"));
    Asset asset2(assets.find_mesh("cube_lq"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"));
    Asset asset3(assets.find_mesh("cube_lq"), assets.find_texture("bricks"));
    Asset monkey_asset(assets.find_mesh("monkey"), assets.find_texture("bricks"));

    Shader gaussian_blur_shader("../../../src/shaders/GaussianBlur");
    CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "cwd", ".jpg");
    Shader skybox_shader("../../../src/shaders/Skybox");
    Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);

    Shader depth_shader("../../../src/shaders/Depth_Instanced");
    FrameBuffer hdr_buffer{wnd.get_width(), wnd.get_height()};
    hdr_buffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, 512, 512, GL_DEPTH_COMPONENT);
    Texture& hdr_texture = hdr_buffer.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    Texture& bloom_texture = hdr_buffer.emplace_texture(GL_COLOR_ATTACHMENT1, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    hdr_buffer.set_output_attachment({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    Panel& hdr_panel = wnd.emplace_child<Panel>(Vector2I{600, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &hdr_texture);
    hdr_panel.uses_hdr = true;
    ShadowMap depth_framebuffer{512, 512};
    // Uncomment this to render the depth texture.
    //wnd.emplace_child<Panel>(Vector2I{0, 600}, Vector2I{300, 300}, &depth_framebuffer.get_depth_texture());
    FrameBuffer bloom_buffer{wnd.get_width(), wnd.get_height()};
    bloom_buffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, 512, 512, GL_DEPTH_COMPONENT);
    Texture& blurred_bloom_texture = bloom_buffer.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    // Uncomment this to render the bloom texture.
    //wnd.emplace_child<Panel>(Vector2I{0, 600}, Vector2I{300, 300}, &bloom_texture);
    wnd.emplace_child<Panel>(Vector2I{0, 600}, Vector2I{300, 300}, &bloom_texture);

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

    StaticObject& monkey = scene.emplace_object(Transform{{}, {}, {100, 100, 100}}, monkey_asset);

    scene.emplace_object(Transform{{-100, -700, 100}, {}, {100, 1, 100}}, asset1);
    scene.emplace_object(Transform{{-200, -1000, 200}, {}, {200, 1, 200}}, asset1);

    long long int time = tz::utility::time::now();
    Timer second_timer;
    TimeProfiler profiler;
	while(!wnd.is_close_requested())
    {
        static float x = 0;
        progress.set_progress((1 + std::sin(x += 0.01)) / 2.0f);
        // play with the HDR exposure and gamma.
        //hdr_gui_shader.set_uniform<float>("exposure", (1.1f + std::sin(x)));
        //hdr_gui_shader.set_uniform<float>("gamma", 1.0f);
        monkey.transform.rotation.y = x;
        //scene.set_point_light(0, {{0, 0, 0}, {0, progress.get_progress(), 1 - progress.get_progress()}, 50000000.0f});
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

        depth_framebuffer.clear(GL_DEPTH_BUFFER_BIT);
        depth_framebuffer.set_render_target();

        Camera light_view = scene.get_directional_light(0).value().get_view();
        render_shader.set_uniform<Matrix4x4>("light_viewprojection", light_view.projection(wnd.get_width(), wnd.get_height()) * light_view.view());
        scene.render(depth_shader, light_view, {wnd.get_width(), wnd.get_height()});

        hdr_buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 0.0f, 0.0f, 0.0f, 0.0f);
        hdr_buffer.set_render_target();
        //wnd.set_render_target();
        //wnd.clear();

        profiler.end_frame();

        // render into the hdr buffer.
        depth_framebuffer.get_depth_texture().bind(&render_shader, 5, "depth_map_sampler");
        scene.render(render_shader, camera, {wnd.get_width(), wnd.get_height()});
        scene.update(delta_time / 1000.0f);

        // dont render the skybox for now.
        //skybox.render(camera, skybox_shader, wnd.get_width(), wnd.get_height());

        bloom_buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 0.0f, 0.0f, 0.0f, 0.0f);
        bloom_buffer.set_render_target();
        // now render a simple quad using the unblurred bloom texture with the gaussian blur shader to blur the bright parts.
        Panel render_panel{Vector2I{0, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &bloom_texture};
        render_panel.render(gaussian_blur_shader, wnd.get_width(), wnd.get_height());
        wnd.set_render_target();
        wnd.clear();
        wnd.update(gui_shader, &hdr_gui_shader);
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