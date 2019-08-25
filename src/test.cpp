#include "core/listener.hpp"
#include "physics/physics.hpp"
#include "graphics/gui/button.hpp"
#include "core/scene.hpp"
#include "graphics/skybox.hpp"
#include "graphics/frame_buffer.hpp"
#include "utility/render.hpp"

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
	Window wnd("Topaz Development Window", 0, 30, 1920, 1080);
	std::cout << "OpenGL debugging enabled: " << wnd.is_opengl_debugging_enabled() << "\n";
	/*
	std::vector<int> ints;
	for(std::size_t i = 0; i < 100; i++)
		ints.push_back(i);
	MemoryPool<int> int_pool{ints};
	for(int i : int_pool)
		std::cout << i << "\n";
	int_pool.default_all();
	for(int i : int_pool)
		std::cout << i << "\n";
	 */

	/*
	AutomaticMemoryPool<int> ints1(10);
	for(int i = 0; i < 10; i++)
		ints1[i] = i;
	for(auto n : ints1)
		std::cout << n << "\n";
	 */

	tz::platform::OGLVertexArray vao;
    tz::platform::OGLVertexBuffer& buf0 = vao.emplace_vertex_buffer();
	tz::platform::OGLVertexBuffer& buf = vao.emplace_vertex_buffer();
    tz::platform::OGLVertexBuffer& buf2 = vao.emplace_vertex_buffer();
    tz::platform::OGLVertexBuffer& buf3 = vao.emplace_vertex_buffer();

    MemoryPool<int> initial_pool = buf0.persistently_map<int>(10, false);
    for(std::size_t i = 0; i < 10; i++)
        initial_pool[i] = static_cast<int>(i);
    // partition the pool.
    auto daughters = tz::utility::memory::partition<int, int, int, int>(initial_pool);
    std::cout << "first svmpool:\n";
    std::cout << daughters.first.get<0>() << "\n";
    std::cout << daughters.first.get<1>() << "\n";
    std::cout << daughters.first.get<2>() << "\n";
    std::cout << "second memorypool:\n";
    for(const auto& i : daughters.second)
    {
        std::cout << i << "\n";
    }
	std::cout << "fin\n";

    MemoryPool<float> float_pool = buf.persistently_map<float>(64, false);
	std::vector<float> float_vec;
	for(std::size_t i = 0; i < 64; i++)
		float_vec.push_back(static_cast<float>(std::pow(i, 3)));
	float_pool = float_vec;
	for(auto& fl : float_pool)
		tz::debug::print(fl, "\n");
    tz::debug::print("float pool marking bitmask:\n");
    float_pool.mark(0, 1);
    float_pool.mark_indices(1, 7, 2);
    float_pool.mark(9, 3);
    float_pool.mark_value(238328, 4);
    float_pool.mark_value(250047, 9);
	for(auto& fl : float_pool)
    {
        tz::debug::print(float_pool.get_value_mark(fl).value_or(0));
    }

	std::cout << "\nsizeof(char) == " << sizeof(char) << "\n";
	//AutomaticDynamicVariadicMemoryPool var_pool{1024};
    DVMPool var_pool = buf2.persistently_map_variadic(1024, false);
	var_pool.push_back<int>(7898);
	var_pool.push_back<double>(1.2049875);
	std::cout << "sizeof<int> == " << sizeof(int) << "\n";
	std::cout << "sizeof<double> == " << sizeof(double) << "\n";
	std::cout << "sizeof<long double> == " << sizeof(long double) << "\n";
	std::cout << "type at index 0 = " << var_pool.get_type_at_index(0).name() << "\n";
	std::cout << "int at index 0 = " << var_pool.at<int>(0) << "\n";
	std::cout << "type at index 1 = " << var_pool.get_type_at_index(1).name() << "\n";
	std::cout << "double at index 1 = " << var_pool.at<double>(1) << "\n";
    var_pool.push_back<double>(1.1287349587);
	std::cout << "double at index 2 = " << var_pool.at<double>(2) << "\n";
	std::cout << "type at index 2 = " << var_pool.get_type_at_index(2).name() << "\n";
	std::cout << var_pool.get_byte_usage() << "/" << var_pool.get_byte_capacity() << " bytes used.\n";
    std::cout << var_pool.get_size() << "/" << " elements used.\n";

    auto svar_pool = buf3.persistently_map_variadic<float, float, int, float>(false);
    svar_pool.get<0>() = 456.349587f;
	svar_pool.get<1>() = 10.5f;
	svar_pool.get<2>() = 0.5f;
    ASVMPool<float, float, int, float> asvar_pool = svar_pool;

    std::cout << svar_pool.get<0>() << "\n";
    std::cout << svar_pool.get<1>() << "\n";
    std::cout << svar_pool.get<2>() << "\n";
    std::cout << svar_pool.get<3>() << "\n";
    buf3.unmap();
    buf3.persistently_map<float>(100, false);
    std::cout << "asvar pool now:\n";
    std::cout << asvar_pool.get<0>() << "\n";
    std::cout << asvar_pool.get<1>() << "\n";
    std::cout << asvar_pool.get<2>() << "\n";
    std::cout << asvar_pool.get<3>() << "\n";

    ASVMPool<float, int, char> s_var_pool{5.0f, 20, 'c'};
    std::cout << s_var_pool.get<float>() << ", " << s_var_pool.get<char>() << "\n";

	wnd.set_debug_callback();
	wnd.set_fullscreen(Window::FullscreenType::WINDOWED_MODE);
	wnd.set_swap_interval_type(Window::SwapIntervalType::IMMEDIATE_UPDATES);

	// During init, enable debug output
	Font font("../res/runtime/fonts/Comfortaa-Regular.ttf", 36);
	Label& label = wnd.emplace_child<Label>(Vector2I{100, 50}, font, Vector3F{0.0f, 0.3f, 0.0f}, "hi");
	ProgressBar& progress = wnd.emplace_child<ProgressBar>(Vector2I{0, 50}, Vector2I{100, 50}, ProgressBarTheme{{{0.5f, {0.0f, 0.0f, 1.0f}}, {1.0f, {1.0f, 0.0f, 1.0f}}}, {0.1f, 0.1f, 0.1f}}, 0.5f);

	KeyListener key_listener(wnd);
	MouseListener mouse_listener(wnd);

	Button& test_button = wnd.emplace_child<Button>(Vector2I{0, 150}, Vector2I{100, 50}, font, Vector3F{}, "press me", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
	Button& wireframe_button = wnd.emplace_child<Button>(Vector2I{0, 100}, Vector2I{100, 50}, font, Vector3F{}, "toggle wireframe", Vector3F{0.1f, 0.1f, 0.1f}, Vector3F{0.8f, 0.8f, 0.8f});
	bool wireframe = false;
	wireframe_button.set_callback([&wireframe](){wireframe = !wireframe;});

	constexpr float speed = 0.5f;
	Shader render_shader("../src/shaders/3D_FullAssetsInstancedShadowsBloom");
	// MDI
	//Shader render_shader("../src/shaders/mdi");

	Shader gui_shader("../src/shaders/Gui");
	Shader hdr_gui_shader("../src/shaders/Gui_HDR");
	Camera camera;
	camera.position = {0, 0, -50};
	Scene scene;
	scene.add_directional_light({{0.5f, 1.0f, 0.0f}, {1, 1, 1}, 0.8f});
	glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Well met.");


	Texture red_texture{Image{std::vector<std::byte>{std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255}}, 1, 1}};
	Texture green_texture{Image{std::vector<std::byte>{std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255}}, 1, 1}};
	Texture blue_texture{Image{std::vector<std::byte>{std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255}}, 1, 1}};
	AssetBuffer assets;
	assets.emplace<Model>("darth_maul", "../res/runtime/models/maul/source/Darth Maul/Darth Maul.dae");
	assets.emplace<Model>("nanosuit", "../res/runtime/models/nanosuit.fbx");
	assets.emplace<Model>("deathwing", "../res/runtime/models/deathwing/Deathwing.fbx");
	assets.emplace<Model>("illidan", "../res/runtime/models/illidan/IllidanLegion.obj");
	assets.emplace<Mesh>("cube", "../res/runtime/models/cube.obj");
	assets.emplace<Mesh>("cube_hd", "../res/runtime/models/cube_hd.obj");
	assets.emplace<Mesh>("monkey", "../res/runtime/models/monkeyhead.obj");
	assets.emplace<Mesh>("cylinder", "../res/runtime/models/cylinder.obj");
	assets.emplace<Mesh>("sphere", "../res/runtime/models/sphere.obj");
	assets.emplace<Mesh>("plane_hd", "../res/runtime/models/plane_hd.obj");
	assets.emplace<Texture>("bricks", "../res/runtime/textures/bricks.jpg");
	assets.emplace<Texture>("stone", "../res/runtime/textures/stone.jpg");
	assets.emplace<Texture>("wood", "../res/runtime/textures/wood.jpg");
	assets.emplace<NormalMap>("bricks_normal", "../res/runtime/normalmaps/bricks_normalmap.jpg");
	assets.emplace<NormalMap>("stone_normal", "../res/runtime/normalmaps/stone_normalmap.jpg");
	assets.emplace<NormalMap>("wood_normal", "../res/runtime/normalmaps/wood_normalmap.jpg");
	assets.emplace<ParallaxMap>("bricks_parallax", "../res/runtime/parallaxmaps/bricks_parallax.jpg");
	assets.emplace<ParallaxMap>("stone_parallax", "../res/runtime/parallaxmaps/stone_parallax.png", 0.06f, -0.5f);
	assets.emplace<ParallaxMap>("wood_parallax", "../res/runtime/parallaxmaps/wood_parallax.jpg");
	assets.emplace<DisplacementMap>("bricks_displacement", "../res/runtime/displacementmaps/bricks_displacement.png", 0.1f);
	//assets.emplace<DisplacementMap>("noise_displacement", tz::graphics::height_map::generate_cosine_noise(256, 256, 100.0f));
	// render noisemap:
	Asset maul(assets.find<Model>("darth_maul"));
	Asset nanosuit(assets.find<Model>("nanosuit"));
	Asset illidan(assets.find<Model>("illidan"));
	Asset deathwing_asset(assets.find<Model>("deathwing"));
	Asset asset0(assets.find<Mesh>("cube_hd"), assets.find_texture("bricks"), assets.find<NormalMap>("bricks_normal"), assets.find<ParallaxMap>("bricks_parallax"), assets.find<DisplacementMap>("bricks_displacement"));
	Asset noise_asset(assets.find<Mesh>("plane_hd"), assets.find_texture("bricks"), assets.find<NormalMap>("bricks_normal"), nullptr, assets.find<DisplacementMap>("noise_displacement"));
	Asset asset1(assets.find_mesh("cube"), assets.find_texture("bricks"), assets.find<NormalMap>("bricks_normal"), assets.find<ParallaxMap>("bricks_parallax"));
	Asset asset2(assets.find_mesh("cube"), assets.find_texture("bricks"), assets.find<NormalMap>("bricks_normal"));
	Asset asset3(assets.find_mesh("cube"), assets.find_texture("bricks"));
	Asset stone_floor(assets.find_mesh("cube"), assets.find_texture("stone"), assets.find<NormalMap>("stone_normal"), assets.find<ParallaxMap>("stone_parallax"));
	Asset wooden_sphere(assets.find_mesh("sphere"), assets.find_texture("wood"), assets.find<NormalMap>("wood_normal"), assets.find<ParallaxMap>("wood_parallax"));
	Asset wooden_cylinder(assets.find_mesh("cylinder"), assets.find_texture("wood"), assets.find<NormalMap>("wood_normal"), assets.find<ParallaxMap>("wood_parallax"));

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
	test_button.set_callback([&scene, &camera, &asset0]()
							 {
								 scene.emplace_object(Transform{camera.position, {}, {10, 10, 10}}, asset0);
								 tz::audio::play_async(AudioClip{"../res/runtime/music/tulips.wav"});
							 });
	std::vector<StaticObject> floor_objects;
	std::vector<DynamicObject> falling_objects;
	constexpr int floor_size = 1500;
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
		using namespace tz::consts::numeric;
		object.add_force({0, -400.0f, 0});
		object.velocity = {rand(-1000.0f, 1000.0f), rand(1000.0f, 2500.0f), rand(-1000.0f, 1000.0f)};
		//object.add_force(Vector3F{0, sine_id * 1.5f, 0});
		object.angular_velocity = {rand(-pi, pi) * 0.1f, rand(-pi, pi) * 0.1f, rand(-pi, pi) * 0.1f};
	}
	scene.emplace<InstancedStaticObject>(floor_objects);
	scene.emplace<InstancedDynamicObject>(falling_objects);
	// add the model objects
	scene.emplace<StaticObject>(Transform{{0, -135, 100}, {}, {50, 50, 50}}, maul);
	scene.emplace<StaticObject>(Transform{{50, -135, 100}, {}, {7, 7, 7}}, nanosuit);
	StaticObject& illidan_object = scene.emplace<StaticObject>(Transform{{-75, -135, 100}, {}, {15, 15, 15}}, illidan);
	StaticObject& deathwing = scene.emplace<StaticObject>(Transform{{0, 200, 0}, {0, 0, 0}, {5, 5, 5}}, deathwing_asset);
	//scene.add_point_light(PointLight{{0, 0, 125}, {1, 1, 1}, 9000.0f});
	scene.emplace<StaticObject>(Transform{{100, 0, 0}, {}, {200, 200, 200}}, wooden_cylinder);
	scene.emplace<StaticObject>(Transform{{0, -50, -70}, {}, {20, 20, 20}}, asset1);
	//scene.emplace<StaticObject>(Transform{{0, -1000, 0}, {}, {4000, 40, 4000}}, noise_asset);
	scene.emplace<DynamicObject>(1.0f, Transform{{0, 100, 0}, {}, {50, 50, 50}}, wooden_cylinder);
	scene.emplace<DynamicObject>(1.0f, Transform{{10, 100, 0}, {}, {100, 50, 50}}, wooden_cylinder);
	scene.emplace<DynamicObject>(1.0f, Transform{{20, 100, 0}, {}, {50, 50, 50}}, wooden_cylinder);
	scene.emplace<DynamicObject>(1.0f, Transform{{40, 100, 0}, {}, {50, 50, 50}}, wooden_cylinder);
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
	scene.emplace<RenderableBoundingBox>(tz::utility::render::see_aabb(assets, scene.get_boundary(), {0.0f, 0.0f, 1.0f}));

	/*
	Scene scene_copy{scene};
	scene_copy.update(0);
	ScenePartitionNode illegal{std::move(*const_cast<ScenePartitionNode*>(scene_copy.get_octree_root())->get_children()[0])};
	 */

	// Try using MDI here.
	RenderableBuffer<StaticObject> static_buffer{0};
	StaticObject deathwing2 = deathwing;
	deathwing2.transform.position.y += 100.0f;
    static_buffer.insert(deathwing2);
	// But just with one object.

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
			using namespace tz::utility::generic::cast;
			label.set_text(to_string(profiler.get_delta_average()) + " ms (" + to_string(profiler.get_fps()) + " fps)");
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
		if(wireframe)
			tz::graphics::enable_wireframe_render(true);
		depth_framebuffer.get_depth_texture().bind(&render_shader, 15, "depth_map_sampler");
		scene.render(main_pass);
		static_buffer.render(main_pass);
		constexpr int tps = 120;
		constexpr float tick_delta = 1000.0f / tps;
		if(tick_timer.millis_passed(tick_delta))
		{
			scene.update(tick_delta / 1000.0f);
			tick_timer.reload();
			static bool done = true;
			static auto rand = LocalRandom{};
			for(const ScenePartitionNode* node : tz::utility::generic::depth_first_search(*scene.get_octree_root()))
			{
				if(!done)
				{
					Vector3F rand_colour = {rand.next_float(0.0f, 1.0f), rand.next_float(0.0f, 1.0f), rand.next_float(0.0f, 1.0f)};
					scene.emplace<RenderableBoundingBox>(tz::utility::render::see_aabb(assets, node->get_region(), rand_colour, 7.5f));
				}
			}
			done = true;
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
		if(key_listener.is_key_pressed("Escape"))
			break;
		if(key_listener.is_key_pressed("W"))
			camera.position += camera.forward() * delta_time * speed;
		if(key_listener.is_key_pressed("S"))
			camera.position += camera.backward() * delta_time * speed;
		if(key_listener.is_key_pressed("A"))
			camera.position += camera.left() * delta_time * speed;
		if(key_listener.is_key_pressed("D"))
			camera.position += camera.right() * delta_time * speed;
		deathwing.transform.rotation.y += 0.01f;
		illidan_object.transform.rotation.y -= 0.01f;
		profiler.end_frame();
	}
}
