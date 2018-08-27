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
    wnd.set_swap_interval_type(Window::SwapIntervalType::LATE_SWAP_TEARING);

    // During init, enable debug output
    Font font("../../../res/runtime/fonts/Comfortaa-Regular.ttf", 36);
    Label& label = wnd.emplace_child<Label>(Vector2I{0, 0}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, Vector3F{0.3f, 0.3f, 0.3f}, 0.5f);

    KeyListener key_listener(wnd);
    MouseListener mouse_listener(wnd);

    Button& test_button = wnd.emplace_child<Button>(Vector2I{0, 150}, Vector2I{100, 50}, font, Vector3F{}, "press me", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    Button& wireframe_button = wnd.emplace_child<Button>(Vector2I{0, 100}, Vector2I{100, 50}, font, Vector3F{}, "toggle wireframe", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
    bool wireframe = false;
    wireframe_button.set_callback([&wireframe](){wireframe = !wireframe;});

    constexpr float speed = 0.5f;
    Shader render_shader("../../../src/shaders/3D_FullAssetsInstancedShadowsBloom");

    Shader gui_shader("../../../src/shaders/Gui");
    Shader hdr_gui_shader("../../../src/shaders/Gui_HDR");
    Camera camera;
    camera.position = {0, 0, -50};
    Scene scene;
    scene.add_directional_light({{0, 1, 0}, {1, 1, 1}, 2.0f});

    AssetBuffer assets;
    assets.emplace<Mesh>("cube_lq", "../../../res/runtime/models/cube.obj");
    assets.emplace<Mesh>("cube", "../../../res/runtime/models/cube_hd.obj");
    assets.emplace<Mesh>("monkey", "../../../res/runtime/models/monkeyhead.obj");
    assets.emplace<Mesh>("cylinder", "../../../res/runtime/models/cylinder.obj");
    assets.emplace<Mesh>("sphere", "../../../res/runtime/models/sphere.obj");
    assets.emplace<Mesh>("plane_hd", "../../../res/runtime/models/plane_hd.obj");
    assets.emplace<Texture>("bricks", "../../../res/runtime/textures/bricks.jpg");
    assets.emplace<Texture>("stone", "../../../res/runtime/textures/stone.jpg");
    assets.emplace<Texture>("wood", "../../../res/runtime/textures/wood.jpg");
    assets.emplace<NormalMap>("bricks_normal", "../../../res/runtime/normalmaps/bricks_normalmap.jpg");
    assets.emplace<NormalMap>("stone_normal", "../../../res/runtime/normalmaps/stone_normalmap.jpg");
    assets.emplace<NormalMap>("wood_normal", "../../../res/runtime/normalmaps/wood_normalmap.jpg");
    assets.emplace<ParallaxMap>("bricks_parallax", "../../../res/runtime/parallaxmaps/bricks_parallax.jpg");
    assets.emplace<ParallaxMap>("stone_parallax", "../../../res/runtime/parallaxmaps/stone_parallax.png", 0.06f, -0.5f);
    assets.emplace<ParallaxMap>("wood_parallax", "../../../res/runtime/parallaxmaps/wood_parallax.jpg");
    assets.emplace<DisplacementMap>("bricks_displacement", "../../../res/runtime/displacementmaps/bricks_displacement.png");
    assets.emplace<DisplacementMap>("noise_displacement", tz::graphics::height_map::generate_cosine_noise(256, 256, 100.0f));
    // render noisemap:
    Asset asset0(assets.find<Mesh>("cube"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), assets.find_parallax_map("bricks_parallax"), assets.find_displacement_map("bricks_displacement"));
    Asset noise_asset(assets.find<Mesh>("plane_hd"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), nullptr, assets.find_displacement_map("noise_displacement"));
    Asset asset1(assets.find_mesh("cube_lq"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"), assets.find_parallax_map("bricks_parallax"));
    Asset asset2(assets.find_mesh("cube_lq"), assets.find_texture("bricks"), assets.find_normal_map("bricks_normal"));
    Asset asset3(assets.find_mesh("cube_lq"), assets.find_texture("bricks"));
    Asset stone_floor(assets.find_mesh("cube_lq"), assets.find_texture("stone"), assets.find_normal_map("stone_normal"), assets.find_parallax_map("stone_parallax"));
    Asset wooden_sphere(assets.find_mesh("sphere"), assets.find_texture("wood"), assets.find_normal_map("wood_normal"), assets.find_parallax_map("wood_parallax"));
    Asset wooden_cylinder(assets.find_mesh("cylinder"), assets.find_texture("wood"), assets.find_normal_map("wood_normal"), assets.find_parallax_map("wood_parallax"));

    Shader gaussian_blur_shader("../../../src/shaders/GaussianBlur");
    Shader gui_bloom_shader("../../../src/shaders/Gui_Bloom");
    CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "cwd", ".jpg");
    Shader skybox_shader("../../../src/shaders/Skybox");
    Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);

    Shader depth_shader("../../../src/shaders/Depth_Instanced");
    FrameBuffer hdr_buffer{wnd.get_width(), wnd.get_height()};
    hdr_buffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, wnd.get_width(), wnd.get_height(), GL_DEPTH_COMPONENT);
    Texture& hdr_texture = hdr_buffer.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    Texture& bloom_texture = hdr_buffer.emplace_texture(GL_COLOR_ATTACHMENT1, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    hdr_buffer.set_output_attachment({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    //Panel& hdr_panel = wnd.emplace_child<Panel>(Vector2I{600, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &hdr_texture);
    //hdr_panel.uses_hdr = true;
    ShadowMap depth_framebuffer{8192, 8192};
    // Uncomment this to render the depth texture.
    //wnd.emplace_child<Panel>(Vector2I{0, 300}, Vector2I{300, 300}, &depth_framebuffer.get_depth_texture());
    FrameBuffer bloom_buffer{wnd.get_width(), wnd.get_height()};
    bloom_buffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, wnd.get_width(), wnd.get_height(), GL_DEPTH_COMPONENT);
    Texture& blurred_bloom_texture = bloom_buffer.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    bloom_buffer.set_output_attachment({GL_COLOR_ATTACHMENT0});
    FrameBuffer bloom_buffer2{wnd.get_width(), wnd.get_height()};
    bloom_buffer2.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, wnd.get_width(), wnd.get_height(), GL_DEPTH_COMPONENT);
    Texture& blurred_bloom_texture2 = bloom_buffer2.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height());
    // Uncomment this to render the bloom texture.
    //wnd.emplace_child<Panel>(Vector2I{0, 300}, Vector2I{300, 300}, &bloom_texture);
    //wnd.emplace_child<Panel>(Vector2I{0, 300}, Vector2I{300, 300}, &blurred_bloom_texture);

    FrameBuffer final_framebuffer{wnd.get_width(), wnd.get_height()};
    final_framebuffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, wnd.get_width(), wnd.get_height(), GL_DEPTH_COMPONENT);
    Texture& output_texture = final_framebuffer.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
    final_framebuffer.set_output_attachment({GL_COLOR_ATTACHMENT0});

    // This is the final panel.
    Panel& window_panel = wnd.emplace_child<Panel>(Vector2I{0, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &output_texture);
    window_panel.uses_hdr = true;
    window_panel.set_local_dimensions_normalised_space({1.0f, 1.0f});

    Random rand;
    test_button.set_callback([&scene, &camera, &asset1]()
                             {
                                 scene.emplace_object(Transform{camera.position, {}, {10, 10, 10}}, asset1);
                                 tz::audio::play_async(AudioClip{"../../../res/runtime/music/tulips.wav"}, 1000.0f);
                             });
    std::vector<StaticObject> floor_objects;
    std::vector<DynamicObject> falling_objects;
    constexpr int floor_size = 925;
    for(float i = 0; i < floor_size; i++)
    {
        int index = static_cast<int>(i);
        int dimensions = std::sqrt(floor_size);
        int row    = index / dimensions;
        int column = index % dimensions;
        const Vector3F scale{20, 1, 20};
        const Vector3F offset{scale * dimensions};
        floor_objects.emplace_back(Transform{Vector3F{(scale.x * row * 2), -100, (scale.z * column * 2)} - offset,
                                       {},
                                       scale}, stone_floor);
        DynamicObject& object = falling_objects.emplace_back(1.0f, Transform{Vector3F{(scale.x * row * 2), -50, (scale.z * column * 2)} - offset,
                                                     {}, scale}, stone_floor);
        //float sine_id = std::abs(std::sin(i / 10));
        float pi = tz::utility::numeric::consts::pi;
        object.add_force({0, -400.0f, 0});
        object.velocity = {rand(-1000.0f, 1000.0f), rand(1000.0f, 2500.0f), rand(-1000.0f, 1000.0f)};
        //object.add_force(Vector3F{0, sine_id * 1.5f, 0});
        object.angular_velocity = {rand(-pi, pi) * 0.1f, rand(-pi, pi) * 0.1f, rand(-pi, pi) * 0.1f};
    }
    scene.emplace<InstancedStaticObject>(floor_objects);
    scene.emplace<InstancedDynamicObject>(falling_objects);
    scene.emplace<StaticObject>(Transform{{0, 0, 0}, {}, {15, 15, 15}}, wooden_sphere);
    scene.emplace<StaticObject>(Transform{{100, 0, 0}, {}, {200, 200, 200}}, wooden_cylinder);
    scene.emplace<StaticObject>(Transform{{0, -50, -70}, {}, {20, 20, 20}}, asset1);
    scene.emplace<StaticObject>(Transform{{0, -1000, 0}, {}, {4000, 40, 4000}}, noise_asset);

    Sprite& example_sprite = scene.emplace<Sprite>(Vector2F{}, 0.0f, Vector2F{100.0f, 100.0f}, assets.find_texture("bricks"));
    DynamicSprite& ex1 = scene.emplace<DynamicSprite>(1.0f, Vector2F{200, 200}, 0.0f, Vector2F{100.0f, 100.0f}, assets.find_texture("bricks"), Vector2F{10.0f, 0.0f});
    ex1.add_force({0.0f, 10.0f});

    long long int time = tz::utility::time::now();
    Timer second_timer, tick_timer;
    TimeProfiler profiler;
    using namespace tz::graphics;
    while(!wnd.is_close_requested())
    {
        static float x = 0;
        progress.set_progress((1 + std::sin(x += 0.01)) / 2.0f);
        //example_sprite.set_rotation(x);
        // play with the HDR exposure and gamma.
        hdr_gui_shader.set_uniform<float>("exposure", 0.4f);
        hdr_gui_shader.set_uniform<float>("gamma", 0.5f);
        //scene.set_point_light(0, {{0, 0, 0}, {0, progress.get_progress(), 1 - progress.get_progress()}, 50000000.0f});
        profiler.begin_frame();
        second_timer.update();
        tick_timer.update();
        if(second_timer.millis_passed(1000.0f))
        {
            using namespace tz::utility::generic::cast;
            label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
            second_timer.reload();
            profiler.reset();
            example_sprite.set_texture(assets.find_texture("stone"));
        }
        long long int delta_time = tz::utility::time::now() - time;
        time = tz::utility::time::now();

        depth_framebuffer.clear(BufferBit::DEPTH);
        depth_framebuffer.set_render_target();

        Camera light_view = scene.get_directional_light(0).value().get_view(scene.get_boundary());
        render_shader.set_uniform<Matrix4x4>("light_viewprojection", light_view.projection(wnd.get_width(), wnd.get_height()) * light_view.view());
        glCullFace(GL_FRONT);
        scene.render(depth_shader, nullptr, light_view, {wnd.get_width(), wnd.get_height()});
        glCullFace(GL_BACK);

        hdr_buffer.clear(BufferBit::COLOUR_AND_DEPTH, 0.0f, 0.0f, 0.0f, 0.0f);
        hdr_buffer.set_render_target();
        //wnd.set_render_target();
        //wnd.clear();

        profiler.end_frame();

        // render into the hdr buffer.
        if(wireframe)
            tz::graphics::enable_wireframe_render(true);
        depth_framebuffer.get_depth_texture().bind(&render_shader, 5, "depth_map_sampler");
        scene.render(render_shader, &gui_shader, camera, {wnd.get_width(), wnd.get_height()});
        constexpr int tps = 50;
        constexpr float tick_delta = 1000.0f / tps;
        if(tick_timer.millis_passed(tick_delta))
        {
            scene.update(tick_delta / 1000.0f);
            tick_timer.reload();
        }
        if(wireframe)
            tz::graphics::enable_wireframe_render(false);

        // dont render the skybox for now.
        //skybox.render(camera, skybox_shader, wnd.get_width(), wnd.get_height());

        // now render a simple quad using the unblurred bloom texture with the gaussian blur shader to blur the bright parts.
        tz::graphics::gui_render_mode();
        Panel render_panel{Vector2I{0, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &bloom_texture};
        constexpr std::size_t blur_factor = 4;
        for(std::size_t i = 0; i < blur_factor; i++)
        {
            bool currently_horizontal = gaussian_blur_shader.get_uniform_value<bool>("horizontal");
            if(currently_horizontal)
            {
                bloom_buffer.set_render_target();
                if(i > 0)
                    render_panel.set_texture(&blurred_bloom_texture2);
                gaussian_blur_shader.set_uniform<bool>("horizontal", false);
            }
            else
            {
                bloom_buffer2.set_render_target();
                if(i > 0)
                    render_panel.set_texture(&blurred_bloom_texture);
                gaussian_blur_shader.set_uniform<bool>("horizontal", true);
            }
            render_panel.render(gaussian_blur_shader, wnd.get_width(), wnd.get_height());
        }
        final_framebuffer.clear(BufferBit::COLOUR, 0.0f, 0.0f, 0.1f, 0.0f);
        final_framebuffer.set_render_target();
        tz::graphics::gui_render_mode();
        Panel another_render_panel{Vector2I{0, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &hdr_texture};
        blurred_bloom_texture.bind(&gui_bloom_shader, 5, "bright_sampler");
        another_render_panel.render(gui_bloom_shader, wnd.get_width(), wnd.get_height());
        wnd.set_render_target();
        wnd.clear();
        wnd.update(gui_shader, &hdr_gui_shader);
        if(mouse_listener.is_left_clicked())
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
        if(key_listener.is_key_pressed("Up"))
            example_sprite.position_screenspace.y += 3;
        if(key_listener.is_key_pressed("Down"))
            example_sprite.position_screenspace.y -= 3;
        if(key_listener.is_key_pressed("Left"))
            example_sprite.position_screenspace.x -= 3;
        if(key_listener.is_key_pressed("Right"))
            example_sprite.position_screenspace.x += 3;
    }
}