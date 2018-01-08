#include "engine.hpp"
#include "listener.hpp"
#include "physics.hpp"
#include "data.hpp"
#include "gui_widget.hpp"

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

class SpawnBlockCommand : public TrivialCommand
{
public:
	SpawnBlockCommand(Engine& engine, std::vector<AABB>& bounds): engine(engine), bounds(bounds){}
	virtual void operator()()
	{
		tz::data::Manager manager(std::string(engine.get_resources().get_raw_file().get_path().data(), engine.get_resources().get_raw_file().get_path().length()));
		std::map<tz::graphics::TextureType, Texture*> textures;
		std::vector<std::string> texture_links = engine.get_resources().get_sequence("textures");
		static Random rand;
		std::size_t random_index = rand.next_int(0, texture_links.size());
		std::string random_texture_link = manager.resource_link(texture_links[random_index]);
		std::string random_normalmap_link = manager.resource_link(texture_links[random_index] + "_normalmap");
		std::string random_parallaxmap_link = manager.resource_link(texture_links[random_index] + "_parallaxmap");
		textures.emplace(tz::graphics::TextureType::TEXTURE, Texture::get_from_link<Texture>(random_texture_link, engine.get_textures()));
		textures.emplace(tz::graphics::TextureType::NORMAL_MAP, Texture::get_from_link<NormalMap>(random_normalmap_link, engine.get_normal_maps()));
		textures.emplace(tz::graphics::TextureType::PARALLAX_MAP, Texture::get_from_link<ParallaxMap>(random_parallaxmap_link, engine.get_parallax_maps()));
		textures.emplace(tz::graphics::TextureType::DISPLACEMENT_MAP, Texture::get_from_link<DisplacementMap>(manager.resource_link("default_displacementmap"), engine.get_displacement_maps()));
		Object3D obj(tz::graphics::find_mesh(manager.resource_link("cube_hd"), engine.get_meshes()), textures, engine.camera.position, engine.camera.rotation, Vector3F(40, 20, 40));
		bounds.push_back(tz::physics::bound_aabb(obj));
		engine.scene.add_object(obj);
	}
	Engine& engine;
	std::vector<AABB>& bounds;
};

void init()
{
	Window wnd(800, 600, "Topaz Development Window");
	Engine engine(&wnd, "../../../res/runtime/properties.mdl");
	
	unsigned int seconds = tz::util::cast::from_string<unsigned int>(engine.get_resources().get_tag("played"));
	float rotational_speed = tz::util::cast::from_string<float>(engine.get_resources().get_tag("rotational_speed"));
	constexpr std::size_t shader_id = 0;
	
	KeyListener key_listener;
	MouseListener mouse_listener;
	engine.register_listener(key_listener);
	engine.register_listener(mouse_listener);
	CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "greenhaze", ".png");
	Shader skybox_shader("../../../src/shaders/skybox");
	
	Timer updater;
	bool noclip = false;
	
	std::vector<AABB> bounds;
	bounds.reserve(engine.scene.get_objects().size());
	for(const Object3D& object : engine.scene.get_objects())
		bounds.push_back(tz::physics::bound_aabb(object));
	
	Vector4F gui_colour(0.0f, 0.0f, 0.0f, 0.95f);
	Font example_font("../../../res/runtime/fonts/CaviarDreams.ttf", 26);
	TextLabel text(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, {}, example_font, "FPS: ...", engine.default_gui_shader);
	TextLabel pos_text(text.get_width(), 0.0f, Vector4F(1, 1, 1, 1), {}, {}, example_font, "Loading...", engine.default_gui_shader);
	TrivialFunctor toggle([&](){text.set_hidden(!text.is_hidden());});
	Panel gui_panel(-1.0f, -1.0f, 1.0f, 1.0f, Vector4F(0.4f, 0.4f, 0.4f, 0.5f), engine.default_gui_shader);
	gui_panel.set_using_proportional_positioning(true);
	gui_panel.set_hidden(true);
	//ExitGuiCommand exit(gui_panel);
	TrivialFunctor exit([&](){gui_panel.set_hidden(!gui_panel.is_hidden());});
	TextLabel gui_title(0.0f, wnd.get_height() - 50, Vector4F(1, 1, 1, 1), gui_colour, {}, example_font, "Main Menu", engine.default_gui_shader);
	Button test_button(0.0f, 2 * text.get_height(), Vector4F(1, 1, 1, 1), gui_colour, {}, example_font, "Hide/Show", engine.default_gui_shader, mouse_listener);
	Button noclip_toggle(0.0f, 2 * text.get_height() + 2 * test_button.get_height(), Vector4F(1, 1, 1, 1), gui_colour, {}, example_font, "Toggle Flight", engine.default_gui_shader, mouse_listener);
	Button spawn_block(0.0f, 2 * text.get_height() + 2 * noclip_toggle.get_height() + 2 * test_button.get_height(), Vector4F(1, 1, 1, 1), gui_colour, {}, example_font, "Spawn Block", engine.default_gui_shader, mouse_listener);
	Button exit_gui_button(wnd.get_width() - 50, wnd.get_height() - 50, Vector4F(1, 1, 1, 1), Vector4F(1.0, 0, 0, 1.0), {}, example_font, " x ", engine.default_gui_shader, mouse_listener);
	Button save_scene_button(0.0f, 2 * text.get_height() + 2 * noclip_toggle.get_height() + 2 * test_button.get_height() + 2 * spawn_block.get_height(), Vector4F(1, 1, 1, 1), gui_colour, {}, example_font, "Save Scene", engine.default_gui_shader, mouse_listener);
	
	TrivialFunctor pop_cmd([](){tz::audio::play_async(AudioClip("../../../res/runtime/music/pop.wav"));});
	wnd.add_child(&text);
	wnd.add_child(&pos_text);
	wnd.add_child(&spawn_block);
	wnd.add_child(&gui_panel);
	gui_panel.add_child(&gui_title);
	gui_panel.add_child(&test_button);
	gui_panel.add_child(&exit_gui_button);
	gui_panel.add_child(&noclip_toggle);
	gui_panel.add_child(&save_scene_button);
	TrivialFunctor save_scene_cmd([&](){const_cast<Scene&>(engine.scene).save();});
	TrivialFunctor toggle_noclip([&](){noclip = !noclip;});
	SpawnBlockCommand spawn_test_cube(engine, bounds);
	test_button.set_on_mouse_click(&toggle);
	test_button.set_on_mouse_over(&pop_cmd);
	exit_gui_button.set_on_mouse_click(&exit);
	exit_gui_button.set_on_mouse_over(&pop_cmd);
	noclip_toggle.set_on_mouse_click(&toggle_noclip);
	noclip_toggle.set_on_mouse_over(&pop_cmd);
	spawn_block.set_on_mouse_click(&spawn_test_cube);
	spawn_block.set_on_mouse_over(&pop_cmd);
	save_scene_button.set_on_mouse_click(&save_scene_cmd);
	save_scene_button.set_on_mouse_over(&pop_cmd);
	
	Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);
	TrivialFunctor render_skybox([&](){skybox.render(engine.camera, skybox_shader, engine.get_meshes(), wnd.get_width(), wnd.get_height());});
	engine.add_update_command(&render_skybox);
	
	Object3D player_object(engine.get_meshes().back().get(), engine.scene.get_objects().front().get_textures(), Vector3F(), Vector3F(), Vector3F(5,5,5));
	TrivialFunctor render_player([&](){if(engine.camera.has_perspective_projection()) return;player_object.render(engine.camera, &(engine.default_shader), wnd.get_width(), wnd.get_height());});
	TrivialFunctor update_player_pos([&](){player_object.position = engine.camera.position;player_object.rotation = engine.camera.rotation;});
	engine.add_update_command(&render_player);
	engine.add_tick_command(&update_player_pos);
	
	bool on_ground = false;
	const float a = 0.5f;
	float speed = 0.0f;
	
	Shader shader_2d("../../../src/shaders/2D");
	
	Sprite test_plane(Vector2F(0.0f, 50.0f), 0.0f, Vector2F(10, 10), engine.scene.get_objects().front().get_textures().at(tz::graphics::TextureType::TEXTURE));
	TrivialFunctor render_2d([&](){test_plane.render(engine.camera, &(shader_2d), wnd.get_width(), wnd.get_height());});
	engine.add_update_command(&render_2d);
	
	FrameBuffer plane_texture_buffer(512, 512);
	Texture& plane_texture = plane_texture_buffer.emplace_texture(GL_COLOR_ATTACHMENT0, 512, 512);
	plane_texture_buffer.emplace_renderbuffer(GL_DEPTH_ATTACHMENT, 512, 512, GL_DEPTH_COMPONENT);
	plane_texture_buffer.set_output_attachment(GL_COLOR_ATTACHMENT0);
	while(!engine.get_window().is_close_requested())
	{
		float multiplier = tz::util::cast::from_string<float>(MDLF(RawFile(engine.get_properties().get_tag("resources"))).get_tag("speed"));
		float velocity = multiplier;
		on_ground = false;
		if(updater.millis_passed(1000))
		{	
			text.set_text("FPS: " + tz::util::cast::to_string(engine.get_fps()));
			pos_text.set_x(text.get_width() * 4);
			Vector3<int> pos_int(engine.camera.position.x, engine.camera.position.y, engine.camera.position.z);
			pos_text.set_text(tz::util::string::format(tz::util::string::devectorise_list_3(Vector3F(pos_int.x, pos_int.y, pos_int.z))));
			updater.reload();
			tz::util::log::message("Window fullscreen: ", wnd.is_fullscreen());
			seconds++;
		}
		
		plane_texture_buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, 0.0f, 0.0f, 0.0f, 0.0f);
		plane_texture_buffer.set_render_target();
		engine.scene.render(engine.camera, &(engine.default_shader), wnd.get_width(), wnd.get_height());
		test_plane = Sprite(Vector2F(0.0f, 300.0f), tz::consts::pi, Vector2F(100, 100), &plane_texture);
		
		if(engine.is_update_due())
		{
			for(const AABB& bound : bounds)
			{
				if(bound.intersects(engine.camera.position))// teleport camera above any object it's inside
				{
					engine.camera.position.y = bound.get_maximum().y;
				}
				if(bound.intersects(engine.camera.position - (Vector3F(0, 1, 0) * (velocity + a))))
					on_ground = true; // todo, teleport player right to the edge (otherwise they might just hover above the point which sucks)
			}
			engine.camera.set_axis_bound(!noclip);
			if(!noclip)
			{
				if(on_ground)
					speed = 0.0f;
				else if(engine.get_time_profiler().get_fps() != 0)
				{
					engine.camera.position -= Vector3F(0, speed, 0);
					speed += a;
				}
			}
			
			if(key_listener.is_key_pressed("Up"))
				engine.camera.fov -= (tz::consts::pi / 200.0f);
			if(key_listener.is_key_pressed("Down"))
				engine.camera.fov += (tz::consts::pi / 200.0f);
			if(key_listener.catch_key_pressed("P"))
				engine.camera.set_has_perspective_projection(!engine.camera.has_perspective_projection());
			if(key_listener.catch_key_pressed("F"))
				wnd.set_fullscreen(!wnd.is_fullscreen() ? Window::FullscreenType::DESKTOP_MODE : Window::FullscreenType::WINDOWED_MODE);
			if(key_listener.is_key_pressed("W"))
			{
				Vector3F after = (engine.camera.position + (engine.camera.forward() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.position = after;
			}
			if(key_listener.is_key_pressed("S"))
			{
				Vector3F after = (engine.camera.position + (engine.camera.backward() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.position = after;
			}
			if(key_listener.is_key_pressed("A"))
			{
				Vector3F after = (engine.camera.position + (engine.camera.left() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.position = after;
			}
			if(key_listener.is_key_pressed("D"))
			{
				Vector3F after = (engine.camera.position + (engine.camera.right() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.position = after;
			}
			if(key_listener.is_key_pressed("Space"))
			{
				Vector3F after = (engine.camera.position + (Vector3F(0, 1, 0) * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.position = after;
			}
			if(key_listener.is_key_pressed("Z"))
			{
				Vector3F after = (engine.camera.position + (Vector3F(0, -1, 0) * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.position = after;
			}
			float angular_speed = tz::util::cast::from_string<float>(engine.get_properties().get_tag("rotational_speed"));
			if(key_listener.is_key_pressed("I"))
			{
				engine.camera.rotation += (Vector3F(1.0f/360.0f, 0, 0) * multiplier * angular_speed * engine.get_time_profiler().get_last_delta());
			}
			if(key_listener.is_key_pressed("K"))
			{
				engine.camera.rotation += (Vector3F(-1.0f/360.0f, 0, 0) * multiplier * angular_speed * engine.get_time_profiler().get_last_delta());
			}
			if(key_listener.is_key_pressed("J"))
			{
				engine.camera.rotation += (Vector3F(0, -1.0f/360.0f, 0) * multiplier * angular_speed * engine.get_time_profiler().get_last_delta());
			}
			if(key_listener.is_key_pressed("L"))
			{
				engine.camera.rotation += (Vector3F(0, 1.0f/360.0f, 0) * multiplier * angular_speed * engine.get_time_profiler().get_last_delta());
			}
			if(key_listener.is_key_pressed("R"))
			{
				engine.camera.position = engine.scene.spawn_point;
				engine.camera.rotation = engine.scene.spawn_orientation;
			}
			if(key_listener.catch_key_pressed("Escape"))
				gui_panel.set_hidden(!gui_panel.is_hidden());
			if(mouse_listener.is_left_clicked() && gui_panel.is_hidden())
			{
				Vector2F delta = mouse_listener.get_mouse_delta_pos();
				engine.camera.rotation.y += rotational_speed * delta.x;
				engine.camera.rotation.x -= rotational_speed * delta.y;
				mouse_listener.reload_mouse_delta();
			}
		}
		exit_gui_button.set_x(wnd.get_width() - (exit_gui_button.get_width() * 2));
		exit_gui_button.set_y(wnd.get_height() - (exit_gui_button.get_height() * 2));
		gui_title.set_y(wnd.get_height() - (gui_title.get_height() * 2));
		updater.update();
		engine.update(shader_id);
	}
	MDLF(engine.get_resources()).edit_tag("played", tz::util::cast::to_string(seconds));
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