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
	init();
	tz::terminate();
	return 0;
}

class FPSToggleCommand : public TrivialCommand
{
public:
	FPSToggleCommand(TextLabel& fps_label): fps_label(fps_label){}
	virtual void operator()(){fps_label.set_hidden(!fps_label.is_hidden());}
	TextLabel& fps_label;
};

class ExitGuiCommand : public TrivialCommand
{
public:
	ExitGuiCommand(Panel& gui_panel): gui_panel(gui_panel){}
	virtual void operator()(){gui_panel.set_hidden(!gui_panel.is_hidden());}
	Panel& gui_panel;
};

class ToggleCommand : public TrivialCommand
{
public:
	ToggleCommand(bool& toggle): toggle(toggle){}
	virtual void operator()(){toggle = !toggle;}
	bool& toggle;
};

class SpawnBlockCommand : public TrivialCommand
{
public:
	SpawnBlockCommand(Engine& engine, std::vector<AABB>& bounds): engine(engine), bounds(bounds){}
	virtual void operator()()
	{
		tz::data::Manager manager(std::string(engine.get_resources().get_raw_file().get_path().data(), engine.get_resources().get_raw_file().get_path().length()));
		std::vector<std::pair<std::string, Texture::TextureType>> textures;
		textures.emplace_back(manager.resource_link("bricks"), Texture::TextureType::TEXTURE);
		textures.emplace_back(manager.resource_link("bricks_normalmap"), Texture::TextureType::NORMAL_MAP);
		textures.emplace_back(manager.resource_link("bricks_parallaxmap"), Texture::TextureType::PARALLAX_MAP);
		textures.emplace_back(manager.resource_link("bricks_displacementmap"), Texture::TextureType::DISPLACEMENT_MAP);
		Object obj(manager.resource_link("cube_hd"), textures, engine.camera.get_position(), engine.camera.get_rotation(), Vector3F(40, 20, 40));
		bounds.push_back(tz::physics::bound_aabb(obj, engine.get_meshes()));
		engine.add_to_world(obj);
	}
	Engine& engine;
	std::vector<AABB>& bounds;
};

class SaveWorldCommand : public TrivialCommand
{
public:
	SaveWorldCommand(World& world): world(world){}
	virtual void operator()()
	{world.save();tz::util::log::message("World Saved.");};
	World& world;
};

class RenderSkyboxCommand : public TrivialCommand
{
public:
	RenderSkyboxCommand(Skybox& skybox, const Camera& camera, Shader& shader, const std::vector<std::unique_ptr<Mesh>>& all_meshes, Window& wnd): skybox(skybox), camera(camera), shader(shader), all_meshes(all_meshes), wnd(wnd){}
	virtual void operator()()
	{
		skybox.render(camera, shader, all_meshes, wnd.get_width(), wnd.get_height());
	}
	Skybox& skybox;
	const Camera& camera;
	Shader& shader;
	const std::vector<std::unique_ptr<Mesh>>& all_meshes;
	Window& wnd;
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
	bounds.reserve(engine.get_world().get_objects().size());
	for(const Object& object : engine.get_world().get_objects())
		bounds.push_back(tz::physics::bound_aabb(object, engine.get_meshes()));
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextLabel text(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "FPS: ...", engine.get_default_gui_shader());
	FPSToggleCommand toggle(text);
	Panel gui_panel(-1.0f, -1.0f, 1.0f, 1.0f, Vector4F(0.4f, 0.4f, 0.4f, 0.5f), engine.get_default_gui_shader());
	gui_panel.set_using_proportional_positioning(true);
	gui_panel.set_hidden(true);
	ExitGuiCommand exit(gui_panel);
	TextLabel gui_title(0.0f, wnd.get_height() - 50, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Main Menu", engine.get_default_gui_shader());
	Button test_button(0.0f, 2 * text.get_height(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.7, 0.7, 1.0), Vector3F(0, 0, 0), example_font, "Hide/Show", engine.get_default_gui_shader(), mouse_listener);
	Button noclip_toggle(0.0f, 2 * text.get_height() + 2 * test_button.get_height(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.7, 0.7, 1.0), Vector3F(0, 0, 0), example_font, "Toggle Flight", engine.get_default_gui_shader(), mouse_listener);
	Button spawn_block(0.0f, 2 * text.get_height() + 2 * noclip_toggle.get_height() + 2 * test_button.get_height(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.5, 0.5, 1.0), Vector3F(), example_font, "Spawn Block", engine.get_default_gui_shader(), mouse_listener);
	Button exit_gui_button(wnd.get_width() - 50, wnd.get_height() - 50, Vector4F(1, 1, 1, 1), Vector4F(1.0, 0, 0, 1.0), Vector3F(0, 0, 0), example_font, "X", engine.get_default_gui_shader(), mouse_listener);
	Button save_world_button(0.0f, 2 * text.get_height() + 2 * noclip_toggle.get_height() + 2 * test_button.get_height() + 2 * spawn_block.get_height(), Vector4F(1, 1, 1, 1), Vector4F(0.7, 0.7, 0.7, 1.0), Vector3F(), example_font, "Save World", engine.get_default_gui_shader(), mouse_listener);
	wnd.add_child(&text);
	wnd.add_child(&spawn_block);
	wnd.add_child(&gui_panel);
	gui_panel.add_child(&gui_title);
	gui_panel.add_child(&test_button);
	gui_panel.add_child(&exit_gui_button);
	gui_panel.add_child(&noclip_toggle);
	gui_panel.add_child(&save_world_button);
	SaveWorldCommand save_world_cmd(const_cast<World&>(engine.get_world()));
	ToggleCommand toggle_noclip(noclip);
	SpawnBlockCommand spawn_test_cube(engine, bounds);
	test_button.set_on_mouse_click(&toggle);
	exit_gui_button.set_on_mouse_click(&exit);
	noclip_toggle.set_on_mouse_click(&toggle_noclip);
	spawn_block.set_on_mouse_click(&spawn_test_cube);
	save_world_button.set_on_mouse_click(&save_world_cmd);
	
	Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);
	RenderSkyboxCommand render_skybox(skybox, engine.camera, skybox_shader, engine.get_meshes(), wnd);
	engine.add_update_command(&render_skybox);
	
	bool on_ground = false;
	const float a = engine.get_world().get_gravity().length();
	float speed = 0.0f;

	while(!engine.get_window().is_close_requested())
	{
		float multiplier = tz::util::cast::from_string<float>(MDLF(RawFile(engine.get_properties().get_tag("resources"))).get_tag("speed"));
		float velocity = multiplier;
		on_ground = false;
		if(updater.millis_passed(1000))
		{
			text.set_text("FPS: " + tz::util::cast::to_string(engine.get_fps()));
			updater.reload();
			seconds++;
		}
		
		if(engine.is_update_due())
		{
			for(const AABB& bound : bounds)
			{
				if(bound.intersects(engine.camera.get_position()))// teleport camera above any object it's inside
				{
					Vector3F position = engine.camera.get_position();
					position.set_y(bound.get_maximum().get_y());
					engine.camera.set_position(position);
				}
				if(bound.intersects(engine.camera.get_position() - (Vector3F(0, 1, 0) * (velocity + a))))
					on_ground = true;
			}
			engine.camera.set_axis_bound(!noclip);
			if(!noclip)
			{
				if(on_ground)
					speed = 0.0f;
				else if(engine.get_time_profiler().get_fps() != 0)
				{
					engine.camera.set_position(engine.camera.get_position() - Vector3F(0, speed, 0));
					speed += a;
				}
			}
			
			if(key_listener.is_key_pressed("W"))
			{
				Vector3F after = (engine.camera.get_position() + (engine.camera.forward() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.set_position(after);
			}
			if(key_listener.is_key_pressed("S"))
			{
				Vector3F after = (engine.camera.get_position() + (engine.camera.backward() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.set_position(after);
			}
			if(key_listener.is_key_pressed("A"))
			{
				Vector3F after = (engine.camera.get_position() + (engine.camera.left() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.set_position(after);
			}
			if(key_listener.is_key_pressed("D"))
			{
				Vector3F after = (engine.camera.get_position() + (engine.camera.right() * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.set_position(after);
			}
			if(key_listener.is_key_pressed("Space"))
			{
				Vector3F after = (engine.camera.get_position() + (Vector3F(0, 1, 0) * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.set_position(after);
			}
			if(key_listener.is_key_pressed("Z"))
			{
				Vector3F after = (engine.camera.get_position() + (Vector3F(0, -1, 0) * velocity));
				bool collide = false;
				for(const AABB& bound : bounds)
				{
					if(bound.intersects(after))
						collide = true;
				}
				if(!collide)
					engine.camera.set_position(after);
			}
			if(key_listener.is_key_pressed("I"))
			{
				engine.camera.set_rotation(engine.camera.get_rotation() + (Vector3F(1.0f/360.0f, 0, 0) * multiplier * 5 * engine.get_time_profiler().get_last_delta()));
			}
			if(key_listener.is_key_pressed("K"))
			{
				engine.camera.set_rotation(engine.camera.get_rotation() + (Vector3F(-1.0f/360.0f, 0, 0) * multiplier * 5 * engine.get_time_profiler().get_last_delta()));
			}
			if(key_listener.is_key_pressed("J"))
			{
				engine.camera.set_rotation(engine.camera.get_rotation() + (Vector3F(0, -1.0f/360.0f, 0) * multiplier * 5 * engine.get_time_profiler().get_last_delta()));
			}
			if(key_listener.is_key_pressed("L"))
			{
				engine.camera.set_rotation(engine.camera.get_rotation() + (Vector3F(0, 1.0f/360.0f, 0) * multiplier * 5 * engine.get_time_profiler().get_last_delta()));
			}
			if(key_listener.is_key_pressed("R"))
			{
				engine.camera.set_position(engine.get_world().get_spawn_point());
				engine.camera.set_rotation(engine.get_world().get_spawn_orientation());
			}
			if(key_listener.catch_key_pressed("Escape"))
				gui_panel.set_hidden(!gui_panel.is_hidden());
			if(mouse_listener.is_left_clicked() && gui_panel.is_hidden())
			{
				Vector2F delta = mouse_listener.get_mouse_delta_pos();
				Vector3F orientation = engine.camera.get_rotation();
				orientation.set_y(orientation.get_y() + (rotational_speed * delta.get_x()));
				orientation.set_x(orientation.get_x() - (rotational_speed * delta.get_y()));
				engine.camera.set_rotation(orientation);
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
	AudioClip test_wav_moved(std::move(test_wav));
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