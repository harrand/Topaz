#include "core/listener.hpp"
#include "physics/physics.hpp"
#include "graphics/gui/button.hpp"
#include "core/scene.hpp"
#include "graphics/skybox.hpp"
#include "graphics/frame_buffer.hpp"
#include "utility/render.hpp"
#include "asteroids.hpp"

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
	Window wnd("Topazteroids!", 0, 30, 1920, 1080);
	std::cout << "OpenGL debugging enabled: " << wnd.is_opengl_debugging_enabled() << "\n";

	wnd.set_debug_callback();
	wnd.set_fullscreen(Window::FullscreenType::WINDOWED_MODE);
	wnd.set_swap_interval_type(Window::SwapIntervalType::IMMEDIATE_UPDATES);

	// During init, enable debug output
	Font font("../res/runtime/fonts/Comfortaa-Regular.ttf", 36);
	Label& label = wnd.emplace_child<Label>(Vector2I{100, 50}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
    Label& asteroid_count_label = wnd.emplace_child<Label>(Vector2I{100, 100}, font, Vector3F{0.0f, 0.3f, 0.0f}, " ");
	ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, ProgressBarTheme{{{0.5f, {0.0f, 0.0f, 1.0f}}, {1.0f, {1.0f, 0.0f, 1.0f}}}, {0.1f, 0.1f, 0.1f}}, 0.5f);

	KeyListener key_listener(wnd);
	MouseListener mouse_listener(wnd);

    AudioMusic bgm{"../res/runtime/music/asteroids.wav"};
    bgm.play();

	bool tabtoggle = false;

	constexpr float speed = 0.1f;
	Shader render_shader("../src/shaders/3D_FullAssetsInstancedShadowsBloom");
	// MDI
	//Shader render_shader("../src/shaders/mdi");

	Shader gui_shader("../src/shaders/Gui");
	Shader hdr_gui_shader("../src/shaders/Gui_HDR");
	Camera camera;
	camera.position = {0, 0, -50};
	Asteroids scene;
	scene.add_directional_light({{0.5f, 1.0f, 0.0f}, {1, 1, 1}, 0.8f});
	glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Well met.");

	Shader gaussian_blur_shader("../src/shaders/GaussianBlur");
	Shader gui_bloom_shader("../src/shaders/Gui_Bloom");
	CubeMap skybox_texture("../res/runtime/textures/skybox/", "cwd", ".jpg");
	Shader skybox_shader("../src/shaders/Skybox");
	Skybox skybox("../res/runtime/models/skybox.obj", skybox_texture);

	Shader depth_shader("../src/shaders/Depth_Instanced");
	FrameBuffer hdr_buffer{wnd.get_width(), wnd.get_height()};
	hdr_buffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, wnd.get_width(), wnd.get_height(), GL_DEPTH_COMPONENT);
	Texture& hdr_texture = hdr_buffer.emplace_texture(GL_COLOR_ATTACHMENT0, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
	Texture& bloom_texture = hdr_buffer.emplace_texture(GL_COLOR_ATTACHMENT1, wnd.get_width(), wnd.get_height(), tz::graphics::TextureComponent::HDR_COLOUR_TEXTURE);
	hdr_buffer.set_output_attachment({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
	//Panel& hdr_panel = wnd.emplace_child<Panel>(Vector2I{0, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &hdr_texture);
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

	//scene.emplace<StaticObject>(Transform{{0, -1000, 0}, {}, {4000, 40, 4000}}, noise_asset);
	/*// BoundaryCluster test...
	BoundaryCluster cluster;
	cluster.emplace_sphere(BoundaryCluster::ClusterIntegration::INTERSECTION, Vector3F{}, 50.0f);
	cluster.emplace_box(BoundaryCluster::ClusterIntegration::UNION, Vector3F{}, Vector3F{100.0f, 100.0f, 100.0f});

	tz::debug::print("does the cluster include point [50, 50, 50]: ", std::boolalpha, cluster.intersects(Vector3F{50.0f, 50.0f, 50.0f}), "\n")*/

	RenderPass main_pass{wnd, {render_shader, gui_shader}, camera};
	RenderPass depth_pass{wnd, {depth_shader}, camera};

	/*
	for(const StaticObject& object : scene.get_static_objects())
	{
		if(object.get_boundary().has_value())
			scene.emplace<RenderableBoundingBox>(tz::utility::render::see_aabb(assets, object.get_boundary().value(), {0.0f, 1.0f, 0.0f}));
	}
	 */
	//scene.emplace<RenderableBoundingBox>(tz::utility::render::see_aabb(assets, scene.get_boundary(), {0.0f, 0.0f, 1.0f}));
    auto spawn_asteroid = [&scene, &rand]()
    {
        auto material_selection = rand.next_int(0, 2);
        AsteroidType type = AsteroidType::IGNEOUS;
        switch(material_selection)
        {
            case 1:
                type = AsteroidType::SEDIMENTARY;
                break;
            case 2:
                type = AsteroidType::METALLIC;
                break;
        }
        float size = rand.next_float(10.0f, 55.0f);
        Asteroid& asteroid = scene.emplace_asteroid({{rand.next_float(-1000.0f, 1000.0f), rand.next_float(-1000.0f, 1000.0f), rand.next_float(-1000.0f, 1000.0f)},
                                                     {rand.next_float(-2.0f, 2.0f), rand.next_float(-2.0f, 2.0f), rand.next_float(-2.0f, 2.0f)},
                                                     {size, size, size}}, type, {}, {rand.next_float(-2.0f, 2.0f), rand.next_float(-2.0f, 2.0f), rand.next_float(-2.0f, 2.0f)});
        asteroid.velocity = {rand.next_float(-50.0f, 50.0f), rand.next_float(-50.0f, 50.0f), rand.next_float(-50.0f, 50.0f)};
    };
    for(std::size_t i = 0; i < 1000; i++)
    {
        spawn_asteroid();
    }
	/*
	Scene scene_copy{scene};
	scene_copy.update(0);
	ScenePartitionNode illegal{std::move(*const_cast<ScenePartitionNode*>(scene_copy.get_octree_root())->get_children()[0])};
	 */
    Asteroid* green_bullet = nullptr;
    Asteroid* red_bullet = nullptr;
    Asteroid* blue_bullet = nullptr;
	long long int time = tz::utility::time::now();
	Timer second_timer, tick_timer;
	TimeProfiler profiler;
	using namespace tz::graphics;
	while(!wnd.is_close_requested())
	{
		profiler.begin_frame();
		static float x = 0;
		progress.set_progress((1 + std::sin(x += 0.01)) / 2.0f);
		//example_sprite.set_rotation(x);
		// play with the HDR exposure and gamma.
		hdr_gui_shader.set_uniform<float>("exposure", 0.7f);
		hdr_gui_shader.set_uniform<float>("gamma", 0.5f);
		//scene.set_point_light(0, {{0, 0, 0}, {0, progress.get_progress(), 1 - progress.get_progress()}, 50000000.0f});
		second_timer.update();
		tick_timer.update();
		if(second_timer.millis_passed(1000.0f))
		{
            spawn_asteroid();
			using namespace tz::utility::generic::cast;
			label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
            asteroid_count_label.set_text(std::string{"asteroid count = "} + to_string(scene.get_number_of<Asteroid>()));
			second_timer.reload();
			profiler.reset();
			//progress.set_visible(!progress.is_visible());
			/*
			std::cout << "total number of objects = " << scene.get_number_of<Renderable>() << "\n";
			std::cout << "number of static objects = " << scene.get_number_of<StaticObject>() << "\n";
			std::cout << "number of dynamic objects = " << scene.get_number_of<DynamicObject>() << "\n";
			std::cout << "number of instanced static objects = " << scene.get_number_of<InstancedStaticObject>() << "\n";
			std::cout << "number of renderable bounding boxes = " << scene.get_number_of<RenderableBoundingBox>() << "\n";
			*/
		}

		long long int delta_time = tz::utility::time::now() - time;
		time = tz::utility::time::now();

		depth_framebuffer.clear(BufferBit::DEPTH);
		depth_framebuffer.set_render_target();

		auto boundary = AABB{{-1000, -1000, -1000}, {1000, 1000, 1000}};//scene.get_boundary();
		Camera light_view = scene.get_directional_light(0).value().get_view(AABB{boundary.get_minimum() / 2.0f, boundary.get_maximum() / 2.0f});
		render_shader.set_uniform<Matrix4x4>("light_viewprojection", light_view.projection(wnd.get_width(), wnd.get_height()) * light_view.view());
		glCullFace(GL_FRONT);
		depth_pass.set_camera(light_view);
		scene.render(depth_pass);
		glCullFace(GL_BACK);

		hdr_buffer.clear(BufferBit::COLOUR_AND_DEPTH, 0.0f, 0.0f, 0.0f, 0.0f);
		hdr_buffer.set_render_target();
		//wnd.set_render_target();
		//wnd.clear();
		// render into the hdr buffer.
		depth_framebuffer.get_depth_texture().bind(&render_shader, 15, "depth_map_sampler");
		scene.render(main_pass);
        if(green_bullet != nullptr)
        {
            scene.set_point_light(0, {green_bullet->transform.position, {0.1f, 1.0f, 0.1f}, 75000.0f});
        }
        if(red_bullet != nullptr)
        {
            scene.set_point_light(1, {red_bullet->transform.position, {1.0f, 0.1f, 0.1f}, 75000.0f});
        }
        if(blue_bullet != nullptr)
        {
            scene.set_point_light(2, {blue_bullet->transform.position, {0.1f, 0.1f, 1.0f}, 75000.0f});
        }
		constexpr int tps = 120;
		constexpr float tick_delta = 1000.0f / tps;
		if(tick_timer.millis_passed(tick_delta))
		{
            if(blue_bullet != nullptr)
            {
                blue_bullet->clear_forces();
                blue_bullet->add_force((camera.position - blue_bullet->transform.position) * 5.0f);
            }
			if(key_listener.catch_key_pressed("G"))
			{
				if(green_bullet != nullptr)
				{
					scene.remove_object(*green_bullet);
					green_bullet = nullptr;
				}
				green_bullet = &scene.emplace_asteroid({camera.position, {rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f)}, {20, 20, 20}}, AsteroidType::GREEN_BULLET, camera.forward() * 500.0f, {rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f)});
				green_bullet->make_collisions = true;
				green_bullet->add_force(camera.forward() * 6500.0f);
                scene.play_shoot();
			}

            if(key_listener.catch_key_pressed("R"))
            {
                if(red_bullet != nullptr)
                {
                    scene.remove_object(*red_bullet);
                    red_bullet = nullptr;
                }
                red_bullet = &scene.emplace_asteroid({camera.position, {rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f)}, {10, 10, 10}}, AsteroidType::RED_BULLET, camera.forward() * 500.0f, {rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f)});
                red_bullet->make_collisions = true;
                red_bullet->add_force(camera.forward() * 6500.0f);
                scene.play_shoot();
            }

            if(key_listener.catch_key_pressed("B"))
            {
                if(blue_bullet != nullptr)
                {
                    scene.remove_object(*blue_bullet);
                    blue_bullet = nullptr;
                }
                blue_bullet = &scene.emplace_asteroid({camera.position, {rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f)}, {10, 10, 10}}, AsteroidType::BLUE_BULLET, camera.forward() * 500.0f, {rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f), rand.next_float(-3.0f, 3.0f)});
                blue_bullet->make_collisions = true;
                blue_bullet->add_force(camera.forward() * 6500.0f);
                scene.play_shoot();
            }

			if(key_listener.catch_key_pressed("Tab"))
				tabtoggle = !tabtoggle;
            if(key_listener.catch_key_pressed("M"))
                bgm.set_paused(!bgm.is_paused());
			scene.update(tick_delta / 1000.0f);
			tick_timer.reload();
			//static bool done = true;
            /**
			for(const ScenePartitionNode* node : tz::utility::generic::depth_first_search(*scene.get_octree_root()))
			{
				if(!done)
				{
					Vector3F rand_colour = {rand.next_float(0.0f, 1.0f), rand.next_float(0.0f, 1.0f), rand.next_float(0.0f, 1.0f)};
					//scene.emplace<RenderableBoundingBox>(tz::utility::render::see_aabb(assets, node->get_region(), rand_colour, 7.5f));
				}
			}
             */
			//done = true;
		}

		// dont render the skybox for now.
		skybox.render(camera, skybox_shader, wnd.get_width(), wnd.get_height());

		// now render a simple quad using the unblurred bloom texture with the gaussian blur shader to blur the bright parts.
		tz::graphics::gui_render_mode();
		Panel render_panel{Vector2I{0, 0}, Vector2I{wnd.get_width(), wnd.get_height()}, &bloom_texture};
		constexpr std::size_t blur_factor = 4;
		for(std::size_t i = 0; i < blur_factor; i++)
		{
			const bool* currently_horizontal_ptr = gaussian_blur_shader.get_uniform_value<bool>("horizontal");
			bool currently_horizontal = false;
			if(currently_horizontal_ptr != nullptr)
				currently_horizontal = *currently_horizontal_ptr;
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
        /*
		if(mouse_listener.is_right_clicked())
		{
			Vector2F position = mouse_listener.get_mouse_position();
			Vector2I position_i{static_cast<int>(position.x), static_cast<int>(position.y)};
			auto found_objects = scene.raycast(position_i, main_pass);
			if(!found_objects.empty())
			{
				std::cout << "found at least one object! there were " << found_objects.size() << "\n";
				std::cout << "rendering its boxes...\n";
				for(const Renderable* found_object : found_objects)
					if(found_object->get_boundary().has_value())
						scene.emplace<RenderableBoundingBox>(tz::utility::render::see_aabb(assets, found_object->get_boundary().value()));
			}
		}
         */
		if(key_listener.is_key_pressed("Escape"))
			break;
		if(key_listener.is_key_pressed("W") || tabtoggle)
			camera.position += camera.forward() * delta_time * speed;
		if(key_listener.is_key_pressed("S"))
			camera.position += camera.backward() * delta_time * speed;
		if(key_listener.is_key_pressed("A"))
			camera.position += camera.left() * delta_time * speed;
		if(key_listener.is_key_pressed("D"))
			camera.position += camera.right() * delta_time * speed;
		profiler.end_frame();
	}
}
